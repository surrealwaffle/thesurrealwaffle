
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits> // std::is_same

#include <sentinel/base.hpp>

#include "types.hpp"

namespace sentinel {
    struct digital_controls_state;
    struct analog_controls_state;
    struct controls_state;
    struct controls_state_ce;

    struct controls_actions;
} // namespace sentinel

namespace reve { namespace controls {

using sentinel::function;
using sentinel::digital_controls_state;
using sentinel::analog_controls_state;
using sentinel::controls_state;
using sentinel::controls_state_ce;
using sentinel::controls_actions;

/** \brief A user filter called to record or modify controls before they are
 *         processed into actions (\ref sentinel::controls_actions).
 *
 * \param[in] digital_controls The state of the user's digital controls.
 * \param[in] analog_controls The state of the user's analog controls.
 * \param[in] seconds The time elapsed since the last render frame, up to `1` second.
 * \param[in] ticks The number of game ticks elapsed for this update.
 */
using controls_filter_type = function<void(digital_controls_state* digital_controls,
                                           analog_controls_state*  analog_controls,
                                           real                    seconds,
                                           ticks_long              ticks)>;

/** \brief Processes controls input from the user by \a user_index into \a actions.
 *
 * This function performs some basic preliminary checks on the game state but
 * otherwise simply translates the controls state (see \ref ptr_ControlsState)
 * into \a actions, with no side-effects.
 *
 * \param[in] user_index The index of the user on this machine (always `0`).
 * \param[in] seconds The time elapsed since the last render frame, up to `1` second.
 * \param[in] ticks The number of game ticks elapsed for this update.
 */
using get_user_actions_tproc __attribute__((cdecl))
    = void(*)(P_IN  int16             user_index,
              P_IN  real              seconds,
              P_OUT controls_actions* actions);

/** \brief Processes controls input from the user by \a user_index with side effects
 *         to the game state.
 *
 * \param[in] user_index The index of the user on this machine (always `0`).
 * \param[in] seconds The time elapsed since the last render frame, up to `1` second.
 * \param[in] ticks The number of game ticks elapsed for this update.
 */
using process_user_controls_tproc __attribute__((cdecl))
    = void(*)(P_IN int16      user_index,
              P_IN real       seconds,
              P_IN int32      ticks);

extern get_user_actions_tproc      proc_GetUserActions;
extern process_user_controls_tproc proc_ProcessUserControls;

extern controls_state*    ptr_ControlsState;
extern controls_state_ce* ptr_ControlsStateCE;

extern digital_controls_state* ptr_DigitalControlsState;
extern analog_controls_state*  ptr_AnalogControlsState;

/** \brief Replacement function for \ref proc_GetUserActions.
 *
 * Calls on \ref proc_GetUserActions to get the character turn responses, then
 * defers to client library filters with the actual turn angles.
 * Finally, calls upon \ref proc_GetUserActions with the updated control states.
 * If the analog turn angles were modified during the second step with the filters,
 * the modified values are set in the final result.
 */
void hook_GetUserActions(P_IN  int16             user_index,
                         P_IN  real              seconds,
                         P_OUT controls_actions* actions)
                         __attribute__((cdecl));

/** \brief Replacement function for \ref proc_ProcessUserControls.
 *
 * Records \a ticks for use in \ref hook_GetUserActions.
 */
void hook_ProcessUserControls(P_IN int16 user_index,
                              P_IN real  seconds,
                              P_IN int32 ticks)
                              __attribute__((cdecl));

/** \brief Installs \a filter to be called during \a hook_GetUserActions in order to
 *         adjust user input or insert additional input.
 *
 * \return A handle to the installation, or
 *         `nullptr` if \a filter could not be installed.
 */
sentinel_handle InstallControlsFilter(controls_filter_type&& filter);

bool Init();

void Debug();

static_assert(std::is_same_v<get_user_actions_tproc,
                             decltype(&hook_GetUserActions)>);
static_assert(std::is_same_v<process_user_controls_tproc,
                             decltype(&hook_ProcessUserControls)>);

} } // namespace reve::controls
