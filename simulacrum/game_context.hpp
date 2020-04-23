
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
#include <sentinel/structures/player.hpp>
#include <sentinel/structures/unit.hpp>
#include <sentinel/types.hpp>

#include <boost/geometry/geometries/box.hpp>

#include "utility.hpp"

namespace simulacrum {

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

    /* unused/unimplemented
    std::optional<weapon_reference>                weapon;
    std::optional<weapon_definition_reference>     weapon_definition;
    std::optional<projectile_definition_reference> projectile_definition;
    */

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

    sentinel::player& get_local_player() const { return local_player.value(); }

    void preupdate(long ticks);

    void postupdate(const sentinel::digital_controls_state& digital);

    static bool load();
};

extern game_context_type game_context;

}
