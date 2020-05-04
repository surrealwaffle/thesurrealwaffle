
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
class ProjectileContext {
private:
    std::reference_wrapper<const sentinel::tags::projectile> projectile_;

    bool destroyed_at_final_velocity_; ///< If `true`, the projectile is destroyed when its speed falls below #velocity_final_.
    bool does_lerp_;               ///< If `true`, the projectile does not lerp.
    sentinel::real velocity_initial_; ///< The muzzle velocity.
    sentinel::real velocity_final_;   ///< The speed at which the projectile stops decelerating.

    sentinel::real detonation_range_; ///< The distance the projectile travels before detonating.
                                      ///< If less than or equal to `0`, the projectile is instead destroyed when it reaches #final_velocity_.
    sentinel::real lerp_distance_; ///< The distance the projectile travels while decelerating from the muzzle.
    sentinel::real lerp_time_;     ///< The number of ticks that velocity is reduced over from the initial velocity.
    sentinel::real lerp_constant_; ///< The rate at which velocity is lost per tick.
    sentinel::real half_lerp_constant_;

public:
    /** \brief Stores various parameters for a given projectile definition.
     */
    ProjectileContext(const sentinel::tags::projectile& projectile) noexcept;

    /** \brief Returns a reference to the projectile definition.
     */
    const sentinel::tags::projectile& projectile() const noexcept { return projectile_; }

    /** \brief Returns the effective maximum range of the projectile.
     */
    sentinel::real max_range() const;

    sentinel::real muzzle_velocity() const { return velocity_initial_; }

    /** \brief Calculates the number of ticks a newly created projectile will take to
     *         travel \a distance.
     *
     * \return The number of ticks, or `std::nullopt` if \a distance is out of range.
     */
    std::optional<sentinel::ticks_long> travel_ticks(sentinel::real       distance,
                                                     std::optional<float> speed  = std::nullopt,
                                                     std::optional<long>  budget = std::nullopt) const;
};

/** \brief Encapsulates partial game state to be used by the AI and control modules.
 */
struct GameContext {
    using PlayerReference = std::reference_wrapper<sentinel::player>;
    using UnitReference   = std::reference_wrapper<sentinel::unit>;
    using WeaponReference = std::reference_wrapper<sentinel::weapon>;
    using WeaponDefinitionReference = std::reference_wrapper<sentinel::tags::weapon>;
    using PlayerReferenceContainer  = std::vector<PlayerReference>;

    std::optional<PlayerReference> local_player;
    std::optional<UnitReference>   local_unit;
    OrientationContext             orientation_context;

    std::optional<WeaponReference>           weapon;
    std::optional<WeaponDefinitionReference> weapon_definition;
    std::optional<ProjectileContext>         projectile_context;

    PlayerReferenceContainer players;
    rough_span<PlayerReferenceContainer::iterator> allies;
    rough_span<PlayerReferenceContainer::iterator> enemies;
    rough_span<PlayerReferenceContainer::iterator> live_allies;  // sorted on distance from player
    rough_span<PlayerReferenceContainer::iterator> live_enemies; // sorted on distance from player

    long ticks_since_fired;
    long ticks_until_can_fire;

    bool can_fire_primary_trigger;
    bool can_fire_secondary_trigger;
    bool can_throw_grenade;

    void preupdate(long ticks);

    void postupdate(const sentinel::digital_controls_state& digital);

    static bool load();

    std::optional<long> projectile_travel_ticks(const sentinel::real& distance,
                                                std::optional<float>  speed  = std::nullopt,
                                                std::optional<long>   budget = std::nullopt);
};


extern GameContext game_context;

}
