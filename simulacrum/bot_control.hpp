#pragma once

#include <cstddef>

#include <array>

#include <sentinel/types.hpp>
#include <sentinel/structures/controls.hpp>
#include <sentinel/structures/player.hpp>

namespace simulacrum { namespace control {

struct immediate_goals_type {
    /** \brief The number of target positions that can be set in #target_position.
     */
    static constexpr std::size_t position_lookahead = 4;

    /** \brief An array of target positions to reach immediately, in sequence.
     *         Multiple positions are supplied at a time for blending.
     *
     * If no positions are set, then no movement takes place.
     */
    std::array<std::optional<sentinel::real3d>, position_lookahead> target_position;

    /** \brief Indicates the player to aim and fire upon (if visible).
     */
    std::optional<std::reference_wrapper<sentinel::player>> target_player;

    /** \brief Sets the goals to a clean, idle state.
     */
    void clear() { *this = {}; }
};

extern immediate_goals_type immediate_goals;

void reset();

/** \brief Installs any filters and callbacks necessary for the AI computations.
 *         IMPORTANT: Call this before \ref simulacrum::control::load().
 */
bool load();

void update(sentinel::digital_controls_state& digital,
            sentinel::analog_controls_state&  analog,
            float seconds,
            long  ticks);

} } // namespace simulacrum::control
