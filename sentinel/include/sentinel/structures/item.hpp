
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__STRUCTURES__ITEM_HPP
#define SENTINEL__STRUCTURES__ITEM_HPP

#include <cstddef> // offsetof

#include <sentinel/types.hpp>
#include <sentinel/structures/object.hpp>

namespace sentinel {

struct item_datum {
    int32 unknown00[14]; // there's stuff here, I just don't know what yet
}; static_assert(sizeof(item_datum) == 0x38);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS

#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

struct item : object {
    item_datum item;
}; static_assert(sizeof(item) == 0x22C);

}

#endif // SENTINEL__STRUCTURES__ITEM_HPP
