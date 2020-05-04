
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "bot_control.hpp"
#include "game_context.hpp"

#include <cmath>
#include <algorithm>

#include "math.hpp"

#include <sentutil/all.hpp>

namespace {

float aiming_fire_threshold = 0.0523599f;
float aiming_delta_factor  = 14.0f;
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
            "affects how quickly the bot can turn",
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

    auto test_target = [&local_player, &unit]
                       (const sentinel::real3d& camera,
                        sentinel::player& target_player)
        -> std::optional<sentinel::real3d> // delta
    {
        if (!target_player.unit)
            return std::nullopt;

        if (game_context.projectile_context) {   // compensate for projectile travel distance
            const ProjectileContext& projectile_context = game_context.projectile_context.value();
            const sentinel::real3d initial_velocity = unit.object.velocity + projectile_context.muzzle_velocity() * unit.unit.aim_forward;
            const sentinel::real initial_projectile_speed = norm(initial_velocity);

            std::optional travel_time = game_context.projectile_travel_ticks(norm(target_player.unit->object.position - camera), initial_projectile_speed, 20L);
            if (!travel_time)
                return std::nullopt; // target cannot be hit
            sentutil::simulation::advance(travel_time.value());
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
                 || opt_raycast_result.value().hit_identity->object.parent == target_player.unit)) {
            return std::nullopt;
        }

        return delta;
    };

    auto aim_to_delta = [seconds, &analog]
                        (const sentinel::real3d& delta) -> bool {
        const auto [dyaw, dpitch] = math::get_turn_angles(game_context.orientation_context, delta);

        analog.turn_left = seconds * aiming_delta_factor * dyaw;
        analog.turn_up   = seconds * aiming_delta_factor * dpitch;

        return std::abs(analog.turn_left) < aiming_fire_threshold
            && std::abs(analog.turn_up)   < aiming_fire_threshold;
    };

    bool do_fire = false;
    [&do_fire, &analog, &unit, local_player, &test_target, &aim_to_delta] { // aiming
        if (!immediate_goals.target_player
            || !immediate_goals.target_player.value().get().unit
            || !game_context.projectile_context)
            return;

        sentinel::player& target_player = immediate_goals.target_player.value();
        sentutil::simulation restore_point;

        sentutil::simulation::advance(1);
        const sentinel::real3d camera = sentutil::object::get_unit_camera(local_player.unit);
        sentutil::simulation::advance(std::max(game_context.ticks_until_can_fire + lead_ticks - 1, 0L));

        for (int i = std::max((int)aiming_lookahead_ticks, 1); i > 0; --i) {
            auto delta = test_target(camera, target_player);
            if (delta) {
                const sentinel::real3d aim_direction
                    = math::approximate_compensated_aim(game_context.projectile_context.value().muzzle_velocity(),
                                                        unit.object.velocity,
                                                        delta.value());
                do_fire = aim_to_delta(aim_direction);
            }

            if (i > 1)
                sentutil::simulation::advance(1);
        }
    }();

    digital.primary_trigger = game_context.can_fire_primary_trigger && do_fire;

    auto signum = [] (const float& f) { return f > 0.05f ? 1.0f : f < -0.05f ? -1.0f : 0.0f; };
    analog.move_forward = signum(analog.move_forward);
    analog.move_left = signum(analog.move_left);
}

} } // namespace simulacrum::control
