
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional>
#include <optional>
#include <utility>

#include <sentinel/structures/controls.hpp>
#include <sentinel/structures/object.hpp>
#include <sentinel/structures/weapon.hpp>
#include <sentinel/structures/player.hpp>
#include <sentinel/structures/unit.hpp>
#include <sentinel/tags/weapon.hpp>
#include <sentinel/tags/projectile.hpp>
#include <sentinel/types.hpp>

#include <boost/geometry/geometries/box.hpp>

#include "bot_config.hpp"
#include "utility.hpp"

namespace simulacrum {

struct OrientationContext {
    OrientationContext() = default;
    OrientationContext(float yaw, float pitch);

    float yaw;
    float pitch;

    float cos_yaw;
    float sin_yaw;

    float cos_pitch;
    float sin_pitch;
};

/** \brief A utility class that captures lead from projectile travel time.
 *
 * Normally, in calculating a Halo-accurate travel time for a projectile, a number of
 * parameters must be calculated, most of which involve a floating point divide.
 * This can be needlessly expensive if this operation is performed for a projectile
 * on multiple data points, so this object precomputes and stores these parameters
 * and their reciprocals when necessary.
 */
struct ProjectileContext {
    std::reference_wrapper<const sentinel::tags::projectile> definition;

    bool destroyed_at_final_speed;
    bool does_lerp;

    sentinel::real        speed_muzzle;
    sentinel::real        speed_final;
    sentinel::real_bounds damage_range;
    sentinel::real        detonation_range;
    sentinel::real        max_range;

    sentinel::real lerp_distance; ///< The distance the projectile lerps over if fired from rest.
    sentinel::real lerp_time;     ///< The time the projectile lerps over if fired from rest.
    sentinel::real lerp_constant; ///< The rate per tick at which speed is reduced if
                                  ///< the projectile's speed is greater than #speed_final.
    sentinel::real reciprocal_lerp_constant; ///< The multiplicative inverse of #lerp_constant.
    sentinel::real half_lerp_constant; ///< Half of #lerp_constant.

    /** \brief Stores various parameters for a given projectile definition.
     */
    ProjectileContext(const sentinel::tags::projectile& projectile) noexcept;
};

/** \brief Encapsulates partial game state to be used by the AI and control modules.
 */
struct GameContext {
    using PlayerReference = std::reference_wrapper<sentinel::player>;
    using UnitReference   = std::reference_wrapper<sentinel::unit>;
    using WeaponReference = std::reference_wrapper<sentinel::weapon>;
    using WeaponDefinitionReference = std::reference_wrapper<sentinel::tags::weapon>;
    using PlayerReferenceContainer  = std::vector<PlayerReference>;
    using WeaponConfigReference = std::reference_wrapper<const config::WeaponConfig>;

    std::optional<PlayerReference> local_player;
    std::optional<UnitReference>   local_unit;
    OrientationContext             orientation_context;

    sentinel::identity<sentinel::weapon>     weapon_id;
    std::optional<WeaponReference>           weapon;
    std::optional<WeaponDefinitionReference> weapon_definition;
    std::optional<ProjectileContext>         projectile_context;
    std::optional<WeaponConfigReference>     weapon_config;

    PlayerReferenceContainer players;
    rough_span<PlayerReferenceContainer::iterator> allies;
    rough_span<PlayerReferenceContainer::iterator> enemies;
    rough_span<PlayerReferenceContainer::iterator> live_allies;  // sorted on distance from player
    rough_span<PlayerReferenceContainer::iterator> live_enemies; // sorted on distance from player

    long ticks_since_fired;

    bool can_fire_primary_trigger;
    bool can_fire_secondary_trigger;
    bool can_throw_grenade;

    void preupdate(long ticks);

    void postupdate(const sentinel::digital_controls_state& digital);

    long get_ticks_until_fire() const;

    const config::WeaponConfig& get_current_weapon_config() const;

    static bool load();
};


extern GameContext game_context;

}
