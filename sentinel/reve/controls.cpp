#include "controls.hpp"

#include <utility> // std::move

#include "globals.hpp"

#include <sentinel/config.hpp>
#include <sentinel/structures/controls.hpp>

namespace {

using sentinel::resource_list;
using reve::controls::controls_filter_type;

// Need: proc_ProcessControls is supplied the correct number of ticks
// but it is not supplied to proc_GetUserActions.
// Furthermore, cumulative ticks cannot be taken from the globals since that
// global has not yet been updated at either hook.
reve::int32 ticks_this_update = 0;

resource_list<controls_filter_type> controls_filters;

} // anonymous

namespace reve { namespace controls {

get_user_actions_tproc      proc_GetUserActions      = nullptr;
process_user_controls_tproc proc_ProcessUserControls = nullptr;

controls_state*    ptr_ControlsState   = nullptr;
controls_state_ce* ptr_ControlsStateCE = nullptr;

digital_controls_state* ptr_DigitalControlsState = nullptr;
analog_controls_state*  ptr_AnalogControlsState  = nullptr;

void hook_GetUserActions(P_IN  int16             user_index,
                         P_IN  real              seconds,
                         P_OUT controls_actions* actions)
{
    // vanilla response without filters
    controls_actions base_response = {/* ZERO INITIALIZED */};
    proc_GetUserActions(user_index, seconds, &base_response);

    // analog_controls_state to measure response from filter
    analog_controls_state filter_analog_response = *ptr_AnalogControlsState;
    filter_analog_response.turn_left = base_response.turn_left;
    filter_analog_response.turn_up   = base_response.turn_up;

    // apply filters
    for (auto& filter : controls_filters)
        filter(ptr_DigitalControlsState,
               &filter_analog_response,
               seconds,
               ticks_this_update);

    // copy back movement to the analog controls state and get the response out to
    // actions
    ptr_AnalogControlsState->move_forward = filter_analog_response.move_forward;
    ptr_AnalogControlsState->move_left    = filter_analog_response.move_left;
    proc_GetUserActions(user_index, seconds, actions);

    // if turn angles were modified, use as supplied by the filter
    if (base_response.turn_left != filter_analog_response.turn_left)
        actions->turn_left = filter_analog_response.turn_left;

    if (base_response.turn_up != filter_analog_response.turn_up)
        actions->turn_up = filter_analog_response.turn_up;
}

void hook_ProcessUserControls(P_IN int16 user_index,
                              P_IN real  seconds,
                              P_IN int32 ticks)
{
    ticks_this_update = ticks;
    proc_ProcessUserControls(user_index, seconds, ticks);
}

sentinel_handle InstallControlsFilter(controls_filter_type&& filter)
{
    return controls_filters.push_back(std::move(filter));
}

bool Init()
{
    ptr_ControlsStateCE = reinterpret_cast<controls_state_ce*>(ptr_ControlsState);

    // set ptr_DigitalControlsState and ptr_AnalogControlsState
    if (ptr_ControlsState && ptr_ControlsStateCE) {
        ptr_DigitalControlsState = edition == GameEdition::combat_evolved ?
            &ptr_ControlsState->digital : &ptr_ControlsStateCE->digital;
        ptr_AnalogControlsState = edition == GameEdition::combat_evolved ?
            &ptr_ControlsState->analog : &ptr_ControlsStateCE->analog;
    }

    return proc_GetUserActions
        && proc_ProcessUserControls
        && ptr_ControlsState
        && ptr_ControlsStateCE
        && ptr_DigitalControlsState
        && ptr_AnalogControlsState;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_GetUserActions);
    SENTINEL_DEBUG_VAR("%p", proc_ProcessUserControls);
    SENTINEL_DEBUG_VAR("%p", ptr_ControlsState);
    SENTINEL_DEBUG_VAR("%p", ptr_ControlsStateCE);
    SENTINEL_DEBUG_VAR("%p", ptr_DigitalControlsState);
    SENTINEL_DEBUG_VAR("%p", ptr_AnalogControlsState);
}

} } // namespace reve::controls
