
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/base.hpp>

#include <sentinel/structures/controls.hpp>

extern "C" {

/** \brief Adds \a filter as a function called when processing user control input.
 *
 * \param[in] filter A pointer to the filter to install. This filter is moved from.
 * \return A handle to the installed filter, or
 *         `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_Controls_InstallControlsFilter(
    sentinel::function<void(sentinel::digital_controls_state* digital_controls,
                            sentinel::analog_controls_state*  analog_controls,
                            sentinel::real                    seconds,
                            sentinel::ticks_long              ticks)>* filter);

} // extern "C""
