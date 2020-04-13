#ifndef SENTINEL__TAGS__PROJECTILE_HPP
#define SENTINEL__TAGS__PROJECTILE_HPP

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
            real maximum_range;    ///< When this value is 0 (as in the plasma rifle), there are wonky effects to the projectile lifetime.
        } detonation;

        struct {
            real air_gravity_scale;
            real_bounds air_damage;

            real water_gravity_scale;
            real_bounds water_damage;

            real initial_velocity; ///< The velocity of the projectile as it is fired, in world units per tick.
            real final_velocity;

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
    }; static_assert(sizeof(projectile) == 0x24C);

} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__PROJECTILE_HPP
