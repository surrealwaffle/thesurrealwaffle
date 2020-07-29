
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "utilities.hpp"

#include <algorithm>
#include <chrono>
#include <optional>
#include <ratio>
#include <string_view>

#include <sentutil/all.hpp>

namespace {

/** @brief Sends @a text over @a channel and displays the time it takes to receive it.
 *
 * Effectively measures application-to-application latency via chat, which may be
 * useful for some servers that disable ping display or for servers which do not
 * appropriately measure this latency such as those running SAPP with the ping
 * measurement enhancements.
 */
void time_chat(std::optional<std::string_view> text,
               std::optional<long>             channel);

bool tantrum_digital(std::string_view input, short period, long hits);

/** @brief If enabled, the user will turn by 180 degrees every tick, but movement
 *         controls remain consistent with relative direction.
 *
 * This command is used to demonstrate that when processing input, turns take effect
 * before directional movement.
 *
 * @param[in] enabled `true` to enable the described behaviour, otherwise
 *                    `false` to disable.
 *                    If supplied an empty optional, no change takes place.
 * \return `true` if this setting is enabled, otherwise `false`.
 */
bool sdbg_flip_move(std::optional<bool> enabled_);

/** @brief Sets the camera to follow a #marker with position relative to that marker.
 *
 * The camera's position is set as an offset from the marker, where the offset is
 * transformed by the parent node transform.
 * The camera is set to point at the marker.
 */
void sdbg_selfie_camera(const char* marker, float x, float y, float z);

void sdbg_disable_selfie_camera();

void chat_filter(long channel, int index, std::wstring_view text);

void controls_filter(sentinel::digital_controls_state& digital,
                     sentinel::analog_controls_state& analog,
                     float seconds,
                     long ticks);

void camera_update(sentinel::camera_globals_type& camera);

float pending_yaw_turn = 0.0f;

bool show_ping = false;

} // namespace (anonymous)

namespace halo_debug {

bool load()
{
    using sentutil::script::install_script_function;
    return install_script_function<"sdbg_time_chat">(time_chat, "measures the time it takes for a string sent over a channel(default=all) to present in chat")
        && install_script_function<"sdbg_tantrum_digital">(tantrum_digital, "hits a key repeatedly periodically", "<string:action> <short:period> <long:count>")
        && install_script_function<"sdbg_turn_yaw">(+[] (float f) { pending_yaw_turn += f; })
        && install_script_function<"sdbg_flip_move">(sdbg_flip_move, "allows for movement in a consistent direction while turning the character 180 degrees every tick")
        && install_script_function<"sdbg_selfie_camera">(sdbg_selfie_camera, "sets the camera to track a local unit's marker")
        && install_script_function<"sdbg_disable_selfie_camera">(sdbg_disable_selfie_camera, "")
        && install_script_function<"sdbg_toggle_show_ping">(+[] () { show_ping = !show_ping; }, "")
        && install_script_function<"sdbg_console_do">(+[] (const char* expression) { sentutil::console::process_expression(expression); })
        && sentutil::chat::install_chat_filter(chat_filter)
        && sentutil::controls::install_controls_filter(controls_filter)
        && sentutil::events::install_camera_update_callback(camera_update);
}

} // namespace halo_debug

namespace {

std::chrono::steady_clock::time_point time_chat_start_time;
std::wstring                          time_chat_pong_string;
void time_chat(std::optional<std::string_view> text,
               std::optional<long>             channel)
{

    time_chat_start_time = std::chrono::steady_clock::now();
    {
        std::string_view t = text.value_or("test");
        time_chat_pong_string.clear();
        // transform is not necessarily correct but close enough
        std::transform(t.begin(), t.end(), std::back_inserter(time_chat_pong_string),
                       [] (char c) { return static_cast<wchar_t>(c); });
    }
    sentutil::chat::send_chat((int)channel.value_or(0L), time_chat_pong_string);
}

sentinel::frames_ubyte sentinel::digital_controls_state::* tantrum_action = nullptr;
short tantrum_period = 0;
long  tantrum_hits = 0;
long  tantrum_ticks_to_hit = 0;
bool tantrum_digital(std::string_view input, short period, long hits)
{
#define TANTRUM_CASE(name) if (input == #name) tantrum_action = &sentinel::digital_controls_state::name
    TANTRUM_CASE(jump);
    else TANTRUM_CASE(switch_grenade);
    else TANTRUM_CASE(action);
    else TANTRUM_CASE(switch_weapons);
    else TANTRUM_CASE(melee);
    else TANTRUM_CASE(flashlight);
    else TANTRUM_CASE(secondary_trigger);
    else TANTRUM_CASE(primary_trigger);
    else TANTRUM_CASE(accept);
    else TANTRUM_CASE(cancel);
    else TANTRUM_CASE(crouch);
    else TANTRUM_CASE(zoom);
    else TANTRUM_CASE(show_scores);
    else TANTRUM_CASE(reload);
    else TANTRUM_CASE(pick_up_weapon);
    else TANTRUM_CASE(say);
    else TANTRUM_CASE(say_team);
    else TANTRUM_CASE(say_vehicle);
    else return (sentutil::console::cprintf({1, 1}, "no action named \"%s\"", input.data()), false);

    period = std::max((short)1, period);
    tantrum_period       = period;
    tantrum_hits         = hits;
    tantrum_ticks_to_hit = period;
    return true;
}

struct {
    bool enabled;
    bool is_flipped;
} flip_move_settings;

bool sdbg_flip_move(std::optional<bool> enabled_)
{
    if (enabled_) {
        const bool enabled = enabled_.value();
        const bool previously_enabled = flip_move_settings.enabled;

        flip_move_settings.enabled = enabled;
        if (enabled && !previously_enabled)
            flip_move_settings.is_flipped = false;
    }

    return flip_move_settings.enabled;
}

struct {
    bool             enabled;
    const char*      marker;
    sentinel::real3d offset;

    sentinel::identity<sentinel::unit> last_unit;
} selfie_camera_info = {false, "", {0, 0, 0}, sentinel::invalid_identity};

void sdbg_selfie_camera(const char* marker, float x, float y, float z)
{
    selfie_camera_info.enabled = true;
    selfie_camera_info.marker  = marker;
    selfie_camera_info.offset  = {x, y, z};
}

void sdbg_disable_selfie_camera()
{
    selfie_camera_info.enabled = false;
    selfie_camera_info.last_unit = sentinel::invalid_identity;
    sentutil::console::process_expression("camera_control 0");
}

void chat_filter(long channel, int index, std::wstring_view text)
{
    const bool is_user = sentutil::globals::players[index].network_index == sentinel_GetLocalPlayerNetworkIndex();

    if (is_user && text == time_chat_pong_string) {
        time_chat_pong_string.clear();
        auto now = std::chrono::steady_clock::now();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(now - time_chat_start_time);
        sentutil::console::cprintf("time_chat result: %ld ms", (long)millis.count());
    }
}

void controls_filter([[maybe_unused]] sentinel::digital_controls_state& digital,
                     [[maybe_unused]] sentinel::analog_controls_state& analog,
                     [[maybe_unused]] float seconds,
                     [[maybe_unused]] long ticks)
{
    // show_ping
    [] {
        if (!show_ping)
            return;

        sentinel::player* local_player = nullptr;
        for (sentinel::player& p : sentutil::globals::players)
            if (p.is_local())
                local_player = &p;
        if (local_player == nullptr)
            return;

        sentutil::console::process_expression("cls");
        sentutil::console::cprintf({1, 0.1, 1, 0.1}, "Ping|t%d", local_player->ping);
    }();

    // pending_yaw_turn
    [&] {
        analog.turn_left += pending_yaw_turn;
        pending_yaw_turn = 0.0f;
    }();

    // sdbg_flip_move
    [&] {
        using sentutil::constants::pi;
        auto& [enabled, is_flipped] = flip_move_settings;

        // sdbg_flip_move must be temporarily enabled while is_flipped to return
        // the user to an unflipped state, otherwise there is a 50/50 chance of
        // leaving the user turned around
        if (!enabled && !is_flipped)
            return;

        if (ticks > 0L) {
            is_flipped = !is_flipped;
            analog.turn_left = is_flipped ? pi : -pi;
        }

        if (is_flipped) {
            analog.move_forward = -analog.move_forward;
            analog.move_left = -analog.move_left;

            digital.secondary_trigger = 0;
            digital.primary_trigger   = 0;
        }
    }();

    if (ticks && tantrum_action != nullptr && tantrum_hits != 0) {
        if ((tantrum_ticks_to_hit -= ticks) <= 0) {
            digital.*tantrum_action = 1;

            while ((tantrum_ticks_to_hit += tantrum_period) <= 0) { /* DO NOTHING */ }
            if (tantrum_hits > 0)
                --tantrum_hits;

            if (tantrum_hits == 0)
                sentutil::console::cprintf({1, 0, 1}, "tantrum done");
        } else {
            digital.*tantrum_action = 0;
        }
    }
}

void camera_update(sentinel::camera_globals_type& camera)
{
    [&] {
        // selfie camera is not enabled, do not track
        if (!selfie_camera_info.enabled)
            return;

        // find the local player in the players table
        sentinel::player* local_player = nullptr;
        for (sentinel::player& player : sentutil::globals::players) {
            if (player.is_local()) {
                local_player = &player;
                break;
            }
        }

        // if the local player does not exist, or they are not alive nor do they have
        // a corpse to track, do nothing
        if (!local_player || !(local_player->unit || local_player->last_unit))
            return;

        const auto unit = local_player->unit ? local_player->unit : local_player->last_unit;

        // attempt to find the marker info
        auto marker_result_opt = sentutil::object::get_object_marker(unit, selfie_camera_info.marker);
        if (!marker_result_opt)
            return; // marker not found

        // if the player respawns, the camera control method gets reset
        // this will set the control method so that the player biped appears properly
        // also plays well with chimera to retain interpolation
        if (selfie_camera_info.last_unit != unit) {
            sentutil::console::process_expression("debug_camera_load");
            selfie_camera_info.last_unit = unit;
        }

        const sentinel::object_marker_result_type& marker = marker_result_opt.value();

        const auto new_position = marker.world_transform * selfie_camera_info.offset;
        camera.position = new_position;
        camera.forward = -marker.world_transform.ortho_transform[0];
        camera.up      = marker.world_transform.ortho_transform[2];
    }();

    [&] {
        if (flip_move_settings.is_flipped) {
            for (int i = 0; i < 2; ++i) {
                camera.forward[i] = -camera.forward[i];
                camera.up[i] = -camera.up[i];
            }
        }
    }();
}

} // namespace (anonymous)
