
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/controls.hpp>

#include "reve/controls.hpp"

SENTINEL_API
sentinel_handle
sentinel_Controls_InstallControlsFilter(
    sentinel::function<void(sentinel::digital_controls_state* digital_controls,
                            sentinel::analog_controls_state*  analog_controls,
                            sentinel::real                    seconds,
                            sentinel::ticks_long              ticks)>* filter)
{

    return filter ? reve::controls::InstallControlsFilter(std::move(*filter))
                  : nullptr;
}
