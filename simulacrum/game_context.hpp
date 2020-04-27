
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

/** \brief A utility class that captures lead from projectile travel time.
 *
 * Normally, in calculating a Halo-accurate travel time for a projectile, a number of
 * parameters must be calculated, most of which involve a floating point divide.
 * This can be needlessly expensive if this operation is performed for a projectile
 * on multiple data points, so this object precomputes and stores these parameters
 * and their reciprocals when necessary.
 */
class projectile_context {
private:
    std::reference_wrapper<const sentinel::tags::projectile> projectile_;

    bool detonates_at_final_velocity_;
    sentinel::real velocity_initial_;
    sentinel::real velocity_final_;
    sentinel::real square_velocity_initial_;
    sentinel::real reciprocal_velocity_final_;

    sentinel::real lerp_distance_;
    sentinel::real lerp_time_;
    sentinel::real lerp_constant_;
    sentinel::real reciprocal_lerp_constant_;

public:
    projectile_context(const sentinel::tags::projectile&) noexcept;

    /** \brief Returns a reference to the projectile definition.
     */
    const sentinel::tags::projectile& projectile() const noexcept { return projectile_; }

    /** \brief Returns the effective maximum range of the projectile.
     */
    sentinel::real max_range() const;

    /** \brief Calculates the number of ticks a newly created projectile will take to
     *         travel \a distance.
     *
     * \return The number of ticks, or `std::nullopt` if \a distance is out of range.
     */
    std::optional<sentinel::ticks_long> travel_ticks(const sentinel::real distance) const;
};

/** \brief Encapsulates partial game state to be used by the AI and control modules.
 */
struct game_context_type {
    using player_reference = std::reference_wrapper<sentinel::player>;
    using unit_reference   = std::reference_wrapper<sentinel::unit>;
    using weapon_reference = std::reference_wrapper<sentinel::weapon>;
    using weapon_definition_reference = std::reference_wrapper<sentinel::tags::weapon>;
    using projectile_definition_reference = std::reference_wrapper<sentinel::tags::projectile>;
    using player_reference_container = std::vector<player_reference>;

    std::optional<player_reference> local_player;
    std::optional<unit_reference>   local_unit;

    std::optional<weapon_reference>                weapon;
    std::optional<weapon_definition_reference>     weapon_definition;
    std::optional<projectile_definition_reference> projectile_definition;
    std::optional<projectile_context>              projectile_context;

    player_reference_container players;
    rough_span<player_reference_container::iterator> allies;
    rough_span<player_reference_container::iterator> enemies;
    rough_span<player_reference_container::iterator> live_allies;  // sorted on distance from player
    rough_span<player_reference_container::iterator> live_enemies; // sorted on distance from player

    long ticks_since_fired;
    long ticks_until_can_fire;

    bool can_fire_primary_trigger;
    bool can_fire_secondary_trigger;
    bool can_throw_grenade;

    void preupdate(long ticks);

    void postupdate(const sentinel::digital_controls_state& digital);

    static bool load();
};


extern game_context_type game_context;

/** \brief Calculates the amount of time (in ticks) required for a \a projectile
 *         to travel \a distance Halo units.
 *
 * \return An optional containing the number of ticks,
 *         or `std::nullopt` if the projectile lacks the range.
 */
std::optional<long>
calculate_projectile_travel_ticks(const float& distance,
                                  sentinel::tags::projectile& projectile);

}
