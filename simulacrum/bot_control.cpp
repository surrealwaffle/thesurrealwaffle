
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "bot_control.hpp"
#include "bot_config.hpp"
#include "game_context.hpp"

#include <cmath>
#include <algorithm>

#include "math.hpp"

#include <sentutil/all.hpp>

namespace simulacrum { namespace control {

immediate_goals_type immediate_goals = {/*ZERO INITIALIZED*/};

void reset()
{
    immediate_goals = immediate_goals_type();
}

bool load()
{
    using sentutil::script::install_script_function;
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
    if (!game_context.local_unit)
        return;

    sentinel::player& local_player = game_context.local_player.value();
    sentinel::unit&   unit         = game_context.local_unit.value();

    sentinel::real3d positional_goal_delta = sentinel::real3d::zero;
    [&analog, &unit, &positional_goal_delta] {   // movement
        const sentinel::unit& controlled_unit = unit.object.parent ? reinterpret_cast<sentinel::unit&>(*unit.object.parent) : unit;

        auto project = [] (const sentinel::real3d& p) -> sentinel::real2d { return {p[0], p[1]}; };
        const sentinel::real2d position = project(controlled_unit.object.position);

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
        positional_goal_delta[0] = weighted_delta[0];
        positional_goal_delta[1] = weighted_delta[1];

        const sentinel::matrix2d body_to_world = {
            sentinel::real2d{controlled_unit.unit.body_forward[0], controlled_unit.unit.body_forward[1]},
            sentinel::real2d{-controlled_unit.unit.body_forward[1], controlled_unit.unit.body_forward[0]}
        };

        const sentinel::matrix2d world_to_body = transpose(body_to_world);
        const auto relative_direction = world_to_body * normalized(weighted_delta);

        analog.move_forward = relative_direction[0];
        analog.move_left = relative_direction[1];
    }();

    auto test_target = [&local_player, &unit]
                       (const sentinel::real3d& camera,
                        sentinel::player& target_player)
        -> std::optional<sentinel::real3d> // delta
    {
        if (!target_player.unit)
            return std::nullopt;

        if (game_context.projectile_context) {   // compensate for projectile travel distance
            const ProjectileContext& projectile_context = game_context.projectile_context.value();
            const sentinel::real3d initial_velocity = math::initial_projectile_velocity(projectile_context.speed_muzzle,
                                                                                        unit.unit.aim_forward,
                                                                                        unit.object.velocity);
            const sentinel::real initial_projectile_speed = norm(initial_velocity);

            const auto [in_range, travel_time] = math::projectile_travel_time(
                game_context.projectile_context.value(),
                initial_projectile_speed,
                norm(target_player.unit->object.node_transforms[0].translation - camera));

            if (!in_range || travel_time > 20.0f)
                return std::nullopt; // target cannot be hit/predicted within budget
            sentutil::simulation::advance(static_cast<long>(travel_time));
        } else {
            return std::nullopt;
        }

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
                 || (target_player.unit->object.parent &&
                     opt_raycast_result.value().hit_identity == target_player.unit->object.parent))) {
            return std::nullopt;
        }

        return delta;
    };

    auto aim_to_delta = [seconds, &analog]
                        (const sentinel::real3d& delta) -> bool {
        const config::AimConfig& aiming = config::get_config_state().aim_config;
        auto turn_amount = [&seconds, &aiming] (const float angle) {
            return -math::decaying_differential(angle,
                                                seconds,
                                                aiming.turn_decay_rate,
                                                aiming.turn_constant_rate);
        };
        const auto [dyaw, dpitch] = math::get_turn_angles(game_context.orientation_context, delta);

        analog.turn_left = std::abs(dyaw)   <= aiming.snap_angle ? dyaw   : turn_amount(dyaw);
        analog.turn_up   = std::abs(dpitch) <= aiming.snap_angle ? dpitch : turn_amount(dpitch);

        const float yaw_remaining   = dyaw - analog.turn_left;
        const float pitch_remaining = dpitch - analog.turn_up;

        return std::abs(yaw_remaining)   < aiming.fire_angle
            && std::abs(pitch_remaining) < aiming.fire_angle;
    };

    bool do_fire = false;
    [&do_fire, &analog, &unit, local_player, &test_target, &aim_to_delta, &positional_goal_delta] { // aiming
        if (!immediate_goals.target_player
            || !immediate_goals.target_player.value().get().unit)
            return;

        sentinel::player& target_player = immediate_goals.target_player.value();
        sentutil::simulation restore_point;

        const auto lead_ticks = config::get_config_state().aim_config.lead_amount;
        sentutil::simulation::advance(1L);
        const sentinel::real3d camera = unit.object.parent ? sentutil::globals::camera_globals->position
                                                           : sentutil::object::get_unit_camera(local_player.unit);
        sentutil::simulation::advance(game_context.get_ticks_until_fire() + lead_ticks - 1L);

        /*for (int i = std::max((int)aiming_lookahead_ticks, 1); i > 0; --i)*/ {
            std::optional<sentinel::real3d> delta = std::nullopt;
            if (game_context.projectile_context && (delta = test_target(camera, target_player))) {
                do_fire = aim_to_delta(delta.value());
            } else {
                aim_to_delta(positional_goal_delta);
            }

            /*
            if (i > 1)
                sentutil::simulation::advance(1);
            */
        }
    }();

    digital.primary_trigger = game_context.can_fire_primary_trigger && do_fire;

    const config::WeaponConfig& weapon_config = game_context.weapon_config.value_or(config::get_config_state().default_weapon_config);
    digital.zoom = weapon_config.preferred_zoom_level >= 0 && weapon_config.preferred_zoom_level != unit.unit.current_zoom_level;

    auto signum = [] (const float& f) { return f > 0.05f ? 1.0f : f < -0.05f ? -1.0f : 0.0f; };
    analog.move_forward = signum(analog.move_forward);
    analog.move_left = signum(analog.move_left);
}

} } // namespace simulacrum::control
