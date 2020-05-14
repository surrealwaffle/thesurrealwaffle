
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/object.hpp>

namespace sentinel { namespace tags {

    struct projectile_definition {
        int32 flags;

        int16 detonation_timer_starts;
        int16 impact_noise;

        int16 function_exports[4];

        tag_reference<void> super_detonation;
        real ai_perception_radius;
        real collision_radius;

        struct {
            real arming_time;
            real danger_radius;
            tag_reference<void> effect;
            real min_timer;
            real max_timer;
            real minimum_velocity;
            real maximum_range;    ///< When this value is 0, the projectile never detonates.
                                   ///< Instead, the projectile is deleted when its speed falls
                                   ///< below or equal to `physics.final_velocity`.
        } detonation;

        struct {
            real        air_gravity_scale; ///< A factor applied to gravity while the projectile is in air.
            real_bounds air_damage_range;  ///< The distance over which damage for the projectile scales in air.

            real water_gravity_scale;       ///< A factor applied to gravity while the projectile is in water.
            real_bounds water_damage_range; ///< The distance over which damage scales for the projectile in water.

            real initial_velocity; ///< The speed of the projectile when created, in world units per tick.
            real final_velocity;   ///< The speed of the projectile is linearly interpolated to this value
                                   ///< AS SOON AS THE PROJECTILE IS CREATED, over a distance `air_damage_range.max - air_damage_range.min` (if in air).
                                   ///< This is potentially a bug in Halo's code.

            real guided_angular_velocity;

            int16 detonation_noise;
            tag_reference<void> detonation_started;
            tag_reference<void> flyby_sound;
            tag_reference<void> detonation_damage;

            int32 : 32;
            int32 : 32;
            int32 : 32;

            tag_reference<void> impact_damage;
        } physics;

        tag_block<void> material_responses;
    }; static_assert(sizeof(projectile_definition) == 0xD0);

    struct projectile : object {
        projectile_definition projectile;

        constexpr bool does_lerp() const noexcept
        {
            return projectile.physics.air_damage_range.length() > 0.0f
                && projectile.physics.initial_velocity != projectile.physics.final_velocity;
        }
    }; static_assert(sizeof(projectile) == 0x24C);

} } // namespace sentinel::tags
