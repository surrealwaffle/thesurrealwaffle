#include "filters.hpp"

#include <algorithm>

#include <sentinel/control.hpp>
#include <sentinel/chat.hpp>
#include <sentinel/types.hpp>
#include <sentinel/player.hpp>
#include <sentinel/raycast.hpp>
#include <sentinel/object.hpp>

#include "program_state.hpp"

namespace {

    sentinel::raycast_result PerspectiveRaycast(sentinel::identity unit);

}

namespace simula::filters {

void OnMapLoad(char const* name) {
    printf("building navigation graph for: [%s]\n", name);
    bool const success = program_state.nav_graph.build();
    printf("\tnavigation graph build: %s\n", success ? "success" : "failure");
}

void OnControlsUpdate(sentinel::control_state& controls,
                      float const dt,
                      long const ticks) {
    if (!program_state.persistent.enabled)
        return;

    program_state.update(dt, ticks);

    // digital actions
    program_state_t::controls_t& sim_controls = program_state.controls;
    for (int i = 0; i < 20; ++i) {
        if (sim_controls.digital_held[i]) {
            controls.digital[i] = std::max(controls.digital[i], sim_controls.state.digital[i]);
        } else if (ticks) {
            controls.digital[i] = std::max(controls.digital[i], sim_controls.state.digital[i]);
            sim_controls.state.digital[i] = 0;
        }
    }

    if (sim_controls.is_moving_forward)
        controls.move_forward = sim_controls.state.move_forward;

    if (sim_controls.is_moving_left)
        controls.move_left = sim_controls.state.move_left;

    if (program_state.targeting.has_target) {
        controls.turn_left = sim_controls.state.turn_left;
        controls.turn_up   = sim_controls.state.turn_up;
    }
}

void OnChatReceive(sentinel::chat_receive_info const& info) {
    if (!program_state.persistent.enabled)
        return;

    sentinel::player_type const& player = sentinel__Player_GetPlayers()->data[info.player_index];
    if (!player.is_valid()) {
        printf("invalid player speaker: %d\n", (int)info.player_index);
        return;
    }

    auto beginswith = [message = info.message, message_len = wcslen(info.message)] (wchar_t const* const str) -> bool {
        if (!str || !message)
            return false;

        std::size_t const len = wcslen(str);
        if (len > message_len)
            return false;

        return wcsncmp(message, str, len) == 0;
    };

    if (beginswith(L"sim stop")) {
        program_state.stop();
    } /*else if (beginswith(L"sim look")) {
        if (!player.unit.is_valid())
            return;

        bool const success = program_state.targeting.set_target(PerspectiveRaycast(player.unit));

        if (!success)
            sentinel::SendChat(info.channel, "I do not know what you are looking at.");
    } */else if (beginswith(L"sim move there")) {
        if (!player.unit.is_valid())
            return;

        auto const raycast = PerspectiveRaycast(player.unit);
        if (raycast.hit_type == -1)
            sentinel::SendChat(info.channel, "I do not know what you are looking at.");
        else if (!program_state.navigation.set_target(program_state, raycast))
            sentinel::SendChat(info.channel, "I cannot move to that location.");
    }
}

}

namespace {

    sentinel::raycast_result PerspectiveRaycast(sentinel::identity unit_id) {
        sentinel::real3 camera {};
        sentinel__Object_GetCameraPosition(unit_id, &camera);

        sentinel::unit_type const *unit = sentinel__Object_GetObjects()->data[unit_id.index()].unit;
        sentinel::real3 delta = unit->aim_forward;
        for (auto&& f : delta)
            f *= 1024;

        sentinel::raycast_result raycast;
        if (!sentinel__Raycast_Test(0x1000ED, &camera, &delta, unit_id, &raycast))
            raycast.hit_type = -1;

        return raycast;
    }

}
