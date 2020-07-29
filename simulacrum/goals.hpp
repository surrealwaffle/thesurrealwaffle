
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <array>
#include <functional>
#include <optional>

#include <sentinel/all.hpp>

// --------------------
// GOAL TYPES AND GOALS
//
// There are two main types of goals:
//  1. Meta goals, which describe the objectives the AI is to plan for.
//  2. Immediate goals, which more closely describe the immediate control input.
//
// Suppose that the meta goal is to attack a particular player (player X).
// Further, suppose that the AI's strategy is to simply get closer to the target, so
// that the target is within nominal firing range.
//
// The meta goal is <Attack player X>.
// The immediate goals are:
//  1. Position: the calculated path to player X (simple strategy).
//  2. Targeting: fire upon player X, if they are within range.
//
// However, meta goals are not so strict.
// If the AI determines that, to get within range of player X, they must attack
// player Y, then the immediate goals change:
//  1. Position: the calculated path to player X.
//  2. Targeting: fire upon player Y, if they are within range.
//
// This represents a very simple plan. A more elaborate strategy would avoid player Y
// while pathing to player X, but this may be too complex for a first implementation.
//
// Why the concept of meta goals?
// They are simple descriptors for directing the bot. For instance, suppose that
// an ally in a vehicle stops near the bot. Rather than explicitly plan out
// "move towards this position and press enter when close enough",
// it is much simpler to specify "enter vehicle/seat from this direction".
//
// Furthermore, the simplification produces a clearer API for controlling the bot
// through the following forms:
//  1. an import library, which would require very little work and would be precise;
//  2. a RESTful API, which requires quite a bit of work on both ends;
//  3. over chat, which would require only parsing/interpreting;
//  4. console commands, which would require very little work.
//
// Methods for server-control over the bot can be particularly interesting, as it
// allows for better coordination between bots present on the server.
// For instance, assuming the player has 2 bots on each team on a game of CTF, the
// server can now direct the bots on each team to capture the flag and/or defend.
//
// Goal entity: A player, object, or position.
// List of meta goals:
//  1. Attack goal entity.
//  2. Defend goal entity.
//  3. Follow goal entity.
//  4. Avoid goal entity.
//  5. Pick up goal entity (requires weapon/equipment goal entity).
//  6. Replace weapon.
//  7. Drop goal entity (requires netgame flag goal entity, e.g. flag and skull).
//  8. Enter goal entity (requires vehicle goal entity).
//  9. Exit current vehicle.

namespace simulacrum {

using GoalEntity = std::variant<
    sentinel::identity<sentinel::player>,
    sentinel::identity<sentinel::object>,
    sentinel::real3d>;

/* BIG-IDEA GOALS
   1. Attack an entity.
   2. Follow an entity.
   3. Defend an entity.
   4. Pick up weapon (optionally replace with specific weapon).
   4.1 Drop netgame flag (at point/direction)
   5. Enter vehicle (seat/direction).
   5.1 Exit vehicle


 */

/** \brief The set of goals describing the immediate input.
 */
struct ImmediateGoal {
    struct {
        static constexpr std::size_t lookahead_amount = 4;

        std::array<std::optional<sentinel::real3d>, lookahead_amount> navs;

        bool                       try_use;
        std::optional<goal_entity> entity;
    } position;

    struct {
        bool try_attack;

        std::optional<goal_entity> entity;
    } target;

    void clear() { *this = {/* ZERO INITIALIZED */}; }
};

}
