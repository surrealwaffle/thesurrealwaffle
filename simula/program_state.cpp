#include "program_state.hpp"

#include <cmath>

#include <sentinel/globals.hpp>
#include <sentinel/object.hpp>
#include <sentinel/types.hpp>
#include <sentinel/raycast.hpp>
#include <sentinel/chat.hpp>

#include "vector_extensions.hpp"

simula::program_state_t simula::program_state;

namespace simula {

    using targeting_t  = program_state_t::targeting_t;
    using navigation_t = program_state_t::navigation_t;

    void targeting_t::update(program_state_t& program_state, float const, long const) {
        constexpr long double PI  = 3.141592653589793238462643383279502884L;
        constexpr long double PI2 = 2L * PI;
        using namespace sentinel;

        if (!has_target)
            return;

        // check if the target still exists
        if (target.is_valid()) {
            if (!sentinel__Object_GetObjects()->data[target.index()].is_valid())
                return (this->clear(), void());
        }

        globals::pc_control const& pc_control = globals::GetPCControl();

        // check if the player is alive
        if (!pc_control.unit.is_valid()) {
            if (program_state.persistent.reset_targeting_on_death)
                this->clear();
            return;
        }

        real3 source_point = sentinel__Globals_GetCamera()->position;
        real3 target_point = target_offset;

        program_state.controls.state.turn_left = 0;
        program_state.controls.state.turn_up   = 0;

        // if the target is an object, target_offset is relative to the object position
        //   and target_point must be corrected
        if (target.is_valid()) {
            object_type *object = sentinel__Object_GetObjects()->data[target.index()].object;

            real3x3 const orientationTransform {object->forward, cross(object->up, object->forward), object->up};
            target_point = mul(orientationTransform, target_point);

            real3 target_world = {0, 0, 0};
            sentinel__Object_GetMarkerPosition(target, target_marker, &target_world);

            for (int i = 0; i < 3; ++i)
                target_point[i] += target_world[i];
        }

        // calculate turning angles
        real3 delta = {};
        for (int i = 0; i < 3; ++i)
            delta[i] = target_point[i] - source_point[i];

        real const& dx = delta[0];
        real const& dy = delta[1];
        real const& dz = delta[2];

        real const horizontal_distance = std::sqrt(dx * dx + dy * dy);
        if (horizontal_distance <= 0.001)
            return; // do not bother turning

        real const yaw   = std::atan2(dy, dx);
        real const pitch = std::atan2(dz, horizontal_distance);

        real delta_yaw   = yaw - pc_control.yaw;
        real delta_pitch = pitch - pc_control.pitch;

        while (delta_yaw < -PI)
            delta_yaw += PI2;

        while (delta_yaw > PI)
            delta_yaw -= PI2;

        // set turning angles
        program_state.controls.state.turn_left = delta_yaw;
        program_state.controls.state.turn_up   = delta_pitch;
    }

    bool targeting_t::set_target(sentinel::raycast_result const& raycast) {
        clear();

        if (raycast.hit_type == 2) { // hit BSP
            has_target    = true;
            target_offset = raycast.terminal;
        } else if (raycast.hit_type == 3) { // hit object
            sentinel::real3 base_position = {0,0,0};
            if (!sentinel__Object_GetMarkerPosition(raycast.hit_identity, "", &base_position)) {
                printf("object %08X had no base marker\n", raycast.hit_identity.raw);
                return false;
            }
            sentinel::object_type *object = sentinel__Object_GetObjects()->data[raycast.hit_identity.index()].object;
            real3x3 const orientationTransform {object->forward, cross(object->up, object->forward), object->up};
            real3x3 const orientationInverseTransform = transpose(orientationTransform);

            has_target = true;
            target     = raycast.hit_identity;
            for (int i = 0; i < 3; ++i)
                target_offset[i] = raycast.terminal[i] - base_position[i];

            target_offset = mul(orientationInverseTransform, target_offset);
        } else {
            return false;
        }

        return true;
    }

    void navigation_t::update(program_state_t& program_state, float const, long const) {
        using namespace sentinel;
        program_state.controls.is_moving_forward = false;
        program_state.controls.is_moving_left = false;

        if (!has_target || surface == -1)
            return;

        globals::pc_control const& pc_control = globals::GetPCControl();

        if (!pc_control.unit.is_valid()) {
            if (program_state.persistent.reset_navigation_on_death)
                this->clear();
            return;
        }

        auto is_surface_navable = [&program_state] (int32 const surface) {
            return surface >= 0 && program_state.nav_graph.surface_map.count(surface);
        };
        biped_type *const player_biped = sentinel__Object_GetObjects()->data[pc_control.unit.index()].biped;
        int32 const surface_index = is_surface_navable(player_biped->cbsp_surface_index) ? player_biped->cbsp_surface_index : last_navable_surface;
        last_navable_surface = surface_index;


        if (surface_index == -1) { // in air, just do nothing for now
            return;
        } else if (program_state.nav_graph.surface_map.count(surface_index) == 0) {
            SendChat("I am on an unmapped surface.");
            this->clear();
            return;
        }

        auto const source_node_index = program_state.nav_graph.surface_map.at(surface_index);
        auto const target_node_index = program_state.nav_graph.surface_map.at(surface);

        if (source_node_index == target_node_index) {
            SendChat("I have arrived.");
            this->clear();
            return;
        }

        if (program_state.nav_graph.distance(source_node_index, target_node_index) >= std::numeric_limits<real>::max()) {
            SendChat("I cannot reach that location from here.");
            this->clear();
            return;
        }

        bool has_path        = false;
        auto next_node_index = source_node_index;
        real next_distance   = program_state.nav_graph.distance(source_node_index, target_node_index);
        for (auto const neighbor_node : program_state.nav_graph.neighbors_of(source_node_index)) {
            real const path_distance = program_state.nav_graph.distance(source_node_index, neighbor_node) + program_state.nav_graph.distance(neighbor_node, target_node_index);

            if (!has_path || path_distance < next_distance) {
                has_path = true;
                next_node_index = neighbor_node;
                next_distance = path_distance;
            }
        }

        real3 source_point = player_biped->position;
        sentinel__Object_GetMarkerPosition(pc_control.unit, "", &source_point);
        real3 const& target_point = program_state.nav_graph.nodes[next_node_index].world.point;
        real3 const delta = sub(target_point, source_point);

        real const& yaw = pc_control.yaw;
        real3 const forward = {cosf(yaw), sinf(yaw), 0};
        real3 const left = {-sinf(yaw), cosf(yaw), 0};

        real const forward_distance = dot(forward, delta);
        real const left_distance    = dot(left, delta);

        constexpr real THRESHOLD = 0.05;

        if (forward_distance < -THRESHOLD || forward_distance > THRESHOLD) {
            program_state.controls.is_moving_forward = true;
            program_state.controls.state.move_forward = forward_distance >= 0 ? 1 : -1;
        }

        if (left_distance < -THRESHOLD || left_distance > THRESHOLD) {
            program_state.controls.is_moving_left = true;
            program_state.controls.state.move_left = left_distance >= 0 ? 1 : -1;
        }
    }

    bool navigation_t::set_target(program_state_t const& program_state, sentinel::raycast_result const& raycast) {
        clear();

        using namespace sentinel;

        if (raycast.hit_type == 2) { // hit BSP
            int32 const surface_index = raycast.surface_index;
            bool const is_in_nav_graph = program_state.nav_graph.surface_map.count(surface_index);

            if (!is_in_nav_graph)
                return false;

            this->has_target = true;
            this->target_offset = raycast.terminal;
            this->surface = surface_index;
        } else {
            return false;
        }

        return true;
    }

}
