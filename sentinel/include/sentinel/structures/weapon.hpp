
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__STRUCTURES__WEAPON_HPP
#define SENTINEL__STRUCTURES__WEAPON_HPP

#include <cstddef> // offsetof

#include <sentinel/types.hpp>
#include <sentinel/structures/item.hpp>

namespace sentinel {

struct weapon_datum {
    struct trigger_datum {
        ticks_byte ticks_since_fire; // 0x00

        int8  unknown00; // 0x01
        int16 unknown01; // 0x02
        int32 unknown02; // 0x04
        int16 unknown03; // 0x08
        int16 unknown04; // 0x0A
        int16 unknown05; // 0x0C
        int16 unknown06; // 0x0E

        fraction rate_of_fire; ///< 0 = maximum rate of fire, 1 = minimum rate of fire
        fraction unknown08; // 0x14
        fraction unknown09; // 0x18
        fraction error; ///< 0 = minimum error, 1 = maximum error

        int32 unknown11;
        int32 unknown12;
    }; static_assert(sizeof(trigger_datum) == 0x28);

    flags_long  flags;
    flags_short owner_flags; // set by the owning unit
    real        unknown00;
    enum_byte   unknown01; // related state, i.e. ready, idle, firing, charging
    ticks_short ticks_until_ready;

    int32 unknown02[9];
    trigger_datum triggers[2];
    int32 unknown03[36];
}; static_assert(sizeof(weapon_datum) == 0x114);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(weapon_datum, triggers) == 0x34);
static_assert(offsetof(weapon_datum, ticks_until_ready) == 0x0E);
static_assert(offsetof(weapon_datum::trigger_datum, ticks_since_fire) == 0x00);
#endif // #ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS

struct weapon : item {
    weapon_datum weapon;
}; static_assert(sizeof(weapon) == 0x340);


}

#endif // SENTINEL__STRUCTURES__WEAPON_HPP
