
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // offsetof

#include <sentinel/types.hpp>

namespace sentinel {

/** \brief Captures the state controls that are either `on` or `off`.
 *
 * Each action may be a value betwen `0` and `255`, indicating the number of
 * consecutive render frames the control has been held down.
 */
struct digital_controls_state {
    frames_ubyte jump;
    frames_ubyte switch_grenade;
    frames_ubyte action;
    frames_ubyte switch_weapons;
    frames_ubyte melee;
    frames_ubyte flashlight;
    frames_ubyte secondary_trigger;
    frames_ubyte primary_trigger;

    frames_ubyte accept; ///< Accepts menu options.
    frames_ubyte cancel; ///< Cancels menu options and opens up the menu.

    frames_ubyte crouch;
    frames_ubyte zoom;
    frames_ubyte show_scores;
    frames_ubyte reload;
    frames_ubyte pick_up_weapon;

    frames_ubyte say;
    frames_ubyte say_team;
    frames_ubyte say_vehicle;

    // not even sure that this contains controls
    frames_ubyte unknown0;
    frames_ubyte unknown1;
}; static_assert(sizeof(digital_controls_state) == 0x14);

/** \brief Captures the state of controls that may vary continuously.
 *
 * Movement controls with values that are not `-1`, `0`, or `1` do not synchronize
 * over multiplayer.
 */
struct analog_controls_state {
    signed_fraction move_forward; ///< The amount to move forward (or backward, if negative) by.
    signed_fraction move_left;    ///< The amount to strafe left (or right, if negative) by.

    radians turn_left; ///< The angle in radians to turn left by.
    radians turn_up;   ///< the angle in radians to turn up by.
}; static_assert(sizeof(analog_controls_state) == 0x10);

/** \brief Captures the state of controls the user has currently input.
 */
struct controls_state {
    digital_controls_state digital;  ///< On or off controls.
    analog_controls_state  analog;   ///< Movement and turning controls.
    boolean                unknown0; ///< I don't know what this is! Can't remember why I think it's a bool either.
}; static_assert(sizeof(controls_state) == 0x28);

/** \brief Captures the state of controls the user has currently input.
 *         This is the structure for the Custom Edition client.
 */
struct controls_state_ce {
    digital_controls_state digital;           ///< On or off controls.
    char                   unknown_stuff[12]; ///< I don't know what this is!
    analog_controls_state  analog;            ///< Movement and turning controls.
    boolean                unknown0;          ///< I don't know what this is! Can't remember why I think it's a bool either.
}; static_assert(sizeof(controls_state_ce) == 0x34);

/** \brief A buffer resulting from filtering a \ref controls_state through
 *         various factors (frame time, stun) and contraints (vector normalization).
 */
struct controls_actions {
    direction2d movement;     ///< Normalized or the zero.
    real        unknown0;     ///< I don't know what this is! Definitely a float.
    radians     turn_left;    ///< The angle to turn left by.
    radians     turn_up;      ///< The angle to turn up by.
    char        unknown1[12]; ///< Digital actions here, not in the same order as in \ref sentinel::digital_control_state though. Uses bits rather than bytes.
}; static_assert(sizeof(controls_actions) == 0x20);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(controls_state, digital) == 0x00);
static_assert(offsetof(controls_state, analog) == 0x14);
static_assert(offsetof(controls_state_ce, digital) == 0x00);
static_assert(offsetof(controls_state_ce, analog) == 0x20);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

}
