
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "bot_control.hpp"
#include "game_context.hpp"

#include <cmath>
#include <algorithm>

#include <sentutil/all.hpp>

namespace {

float aiming_fire_threshold = 0.0523599f;
float aiming_delta_factor  = 20.0f;
short aiming_lookahead_ticks = 0;
short lead_ticks = 0;

} // namepace (anonymous)

namespace simulacrum { namespace control {

immediate_goals_type immediate_goals = {/*ZERO INITIALIZED*/};

void reset()
{
    immediate_goals = immediate_goals_type();
}

bool load()
{
    using sentutil::script::install_script_function;
    return
        install_script_function<"simulacrum_aiming_fire_threshold">(
            +[] (std::optional<float> f) {
                if (f) aiming_fire_threshold = f.value();
                return aiming_fire_threshold;
            },
            "the angle (in radians) at which the bot is allowed to fire on targets",
            "[<real>]"
        ) &&
        install_script_function<"simulacrum_aiming_delta_factor">(
            +[] (std::optional<float> f) {
                if (f) aiming_delta_factor = f.value();
                return aiming_delta_factor;
            },
            "effects how quickly the bot can turn",
            "[<real>]"
        ) &&
        install_script_function<"simulacrum_aiming_lookahead_ticks">(
            +[] (std::optional<short> v) {
                if (v) aiming_lookahead_ticks = v.value();
                return aiming_lookahead_ticks;
            },
            "determines how many ticks the bot can check for testing visibility",
            "[<short>]"
        ) &&
        install_script_function<"simulacrum_aiming_lead_ticks">(
            +[] (std::optional<short> v) {
                if (v) lead_ticks = v.value();
                return lead_ticks;
            },
            "determines how many ticks to lead by",
            "[<short>]"
        );
    return true;
}

void update(sentinel::digital_controls_state& digital,
            sentinel::analog_controls_state&  analog,
            float seconds,
            long  ticks)
{
    using simulacrum::control::immediate_goals;

    static bool last_alive = false;
    const bool alive = static_cast<bool>(game_context.local_unit);
    [[maybe_unused]] const bool revived = alive && !last_alive;
    last_alive = alive;
    //digital.flashlight = revived;
    if (!game_context.local_unit)
        return;

    sentinel::player& local_player = game_context.local_player.value();
    sentinel::unit&   unit         = game_context.local_unit.value();

    if ((unit.unit.flags & 0x80000) == 0)
        digital.flashlight = 1;

    [&analog, &unit] {   // movement
        auto project = [] (const sentinel::real3d& p) -> sentinel::real2d
                       { return {p[0], p[1]}; };
        const sentinel::real2d position = project(unit.object.position);

        sentinel::real2d weighted_delta = sentinel::real2d::zero;
        float weight = 1.0f;
        for (const auto& p : simulacrum::control::immediate_goals.target_position) {
            if (!p)
                break;

            const auto goal_delta = project(p.value()) - position;
            if (norm2(goal_delta) > 0.05f) {
                weighted_delta += weight * goal_delta;
                weight *= (1.0f / 4.0f);
            }
        }

        const sentinel::matrix2d body_to_world = {
            sentinel::real2d{unit.unit.body_forward[0], unit.unit.body_forward[1]},
            sentinel::real2d{-unit.unit.body_forward[1], unit.unit.body_forward[0]}
        };

        const sentinel::matrix2d world_to_body = transpose(body_to_world);
        const auto relative_direction = world_to_body * normalized(weighted_delta);

        analog.move_forward = relative_direction[0];
        analog.move_left = relative_direction[1];
    }();

    auto test_target = [local_player]
                       (const sentinel::real3d& camera,
                        sentinel::player& target_player)
        -> std::optional<sentinel::real3d> // delta
    {
        if (!target_player.unit)
            return std::nullopt;
        auto opt_marker_result = sentutil::object::get_object_marker(target_player.unit, "body");
        if (!opt_marker_result)
            return std::nullopt;

        const sentinel::real3d target = opt_marker_result.value().world_transform.translation;
        const sentinel::real3d delta  = target - camera;

        auto opt_raycast_result = sentutil::raycast::cast_projectile_ray(camera,
                                                                         delta,
                                                                         local_player.unit);
        if (!opt_raycast_result
            || opt_raycast_result.value().hit_type != 3
            || !(opt_raycast_result.value().hit_identity == target_player.unit
                 || opt_raycast_result.value().hit_identity->object.parent == target_player.unit)) {
            return std::nullopt;
        }

        return delta;
    };

    const sentinel::matrix2d horizontal_orient_world_to_camera = [] {
        const float yaw = sentutil::globals::local_player_globals->players[0].yaw;
        const auto cos_yaw = std::cos(yaw);
        const auto sin_yaw = std::sin(yaw);

        const sentinel::matrix2d orient_camera_to_world {
            cos_yaw, sin_yaw,
            -sin_yaw, cos_yaw
        };
        return transpose(orient_camera_to_world);
    }();

    const sentinel::matrix2d vertical_orient_world_to_camera = [] {
        const float pitch = sentutil::globals::local_player_globals->players[0].pitch;
        const auto cos_pitch = std::cos(pitch);
        const auto sin_pitch = std::sin(pitch);

        const sentinel::matrix2d orient_camera_to_world {
            cos_pitch, sin_pitch,
            -sin_pitch, cos_pitch
        };
        return transpose(orient_camera_to_world);
    }();

    auto aim_to_delta = [seconds,
                         &analog,
                         horizontal_orient_world_to_camera,
                         vertical_orient_world_to_camera]
                        (const sentinel::real3d& delta) -> bool {
        sentinel::real2d delta_xy = {delta[0], delta[1]};
        sentinel::real2d delta_z  = {norm(delta_xy), delta[2]};

        delta_xy = horizontal_orient_world_to_camera * delta_xy;
        delta_z  = vertical_orient_world_to_camera * delta_z;

        const float yaw_to_target   = std::atan2(delta_xy[1], delta_xy[0]);
        const float delta_yaw       = seconds * aiming_delta_factor * yaw_to_target;

        const float pitch_to_target = std::atan2(delta_z[1], delta_z[0]);
        const float delta_pitch     = seconds * aiming_delta_factor * pitch_to_target;

        analog.turn_left = delta_yaw;
        analog.turn_up   = delta_pitch;

        return std::abs(delta_yaw) < aiming_fire_threshold
            && std::abs(delta_pitch) < aiming_fire_threshold;
    };

    bool do_fire = false;
    [&do_fire, &analog, &unit, local_player, &test_target, &aim_to_delta] { // aiming
        if (!immediate_goals.target_player
            || !immediate_goals.target_player.value().get().unit)
            return;

        sentinel::player& target_player = immediate_goals.target_player.value();
        sentutil::simulation restore_point;

        sentutil::simulation::advance(1);
        const sentinel::real3d camera = sentutil::object::get_unit_camera(local_player.unit);
        sentutil::simulation::advance(game_context.ticks_until_can_fire + lead_ticks);

        {
            auto delta = test_target(camera, target_player);
            if (delta) {
                do_fire = aim_to_delta(delta.value());
            }
        }

        for (int i = aiming_lookahead_ticks; i > 0; --i) {
            auto delta = test_target(camera, target_player);
            if (delta) {
                aim_to_delta(delta.value());
                break;
            }

            if (i > 1)
                sentutil::simulation::advance(1);
        }
    }();

    digital.primary_trigger = game_context.can_fire_primary_trigger && do_fire;
}

} } // namespace simulacrum::control
