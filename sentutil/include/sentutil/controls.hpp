
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include <sentinel/controls.hpp>

namespace sentutil { namespace controls {

/** \brief Installs a filter for control input processing.
 *
 * The filter is invoked with:
 *  * a reference to the analog control state,
 *  * a reference to the digital controls state,
 *  * the number of seconds since the last render frame,
 *  * the number of game ticks in this cumulative update,
 * in that order.
 *
 * \param[in] f The filter to install.
 */
template<class Filter>
std::enable_if_t<
    std::is_invocable_v<
        Filter&,
        sentinel::digital_controls_state& /*digital_controls*/,
        sentinel::analog_controls_state&  /*analog_controls*/,
        sentinel::real                    /*seconds*/,
        sentinel::ticks_long              /*ticks*/>,
bool> install_controls_filter(Filter f)
{
    sentinel::function<void(sentinel::digital_controls_state* digital_controls,
                            sentinel::analog_controls_state*  analog_controls,
                            sentinel::real                    seconds,
                            sentinel::ticks_long              ticks)> filter(
        [f] (auto* digital, auto* analog, auto seconds, auto ticks)
        { (void)f(*digital, *analog, seconds, ticks); }
    );
    return utility::manage_handle(sentinel_Controls_InstallControlsFilter(&filter));
}

} } // namespace sentutil::control

