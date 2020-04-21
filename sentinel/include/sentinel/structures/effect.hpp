
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/tags/effect.hpp>

namespace sentinel {

struct effect {
    identity_salt salt;
    int16         unknown00;
    identity<tags::effect>      tag;
    int32         unknown01[61];
}; static_assert(sizeof(effect) == 0xFC);

} // namespace sentinel
