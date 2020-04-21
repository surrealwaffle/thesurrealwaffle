
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__STRUCTURES__UNIT_HPP
#define SENTINEL__STRUCTURES__UNIT_HPP

#include <cstddef> // offsetof

#include <sentinel/types.hpp>
#include <sentinel/fwd/table_fwd.hpp>
#include <sentinel/structures/object.hpp>

namespace sentinel {

struct unit_datum {
    identity_raw unknown00;
    identity_raw unknown01;
    identity_raw unknown02;
    identity_raw unknown03;

    flags_long flags;
    flags_long more_flags;

    int32 unknown04[6];

    direction3d body_forward; ///< The forward direction of the body.
    direction3d aim_forward;  ///< The forward direction of the unit's aim.

    int32 unknown05[15];

    real3d movement;

    int32 unknown06[27];

    index_short seat_index; ///< The index of the vehicle seat the unit is occupying, or `-1` if the unit is not in a vehicle.

    index_short weapon_index;         ///< The index of the weapon in #weapons the unit is using, or `-1` if the unit is not using a weapon in #weapons.
    index_short desired_weapon_index; ///< The index of the weapon in #weapons the unit is trying to use, even when the user is in the gunner seat.
    identity<weapon> weapons[4];      ///< The weapons the unit has available, including the flag and oddball.

    int32 unknown07[113];
}; static_assert(sizeof(unit_datum) == 0x2D8);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(unit_datum, body_forward) == 0x30);
static_assert(offsetof(unit_datum, aim_forward) == 0x3C);
static_assert(offsetof(unit_datum, movement) == 0x84);
static_assert(offsetof(unit_datum, seat_index) == 0xFC);
static_assert(offsetof(unit_datum, weapon_index) == 0xFE);
static_assert(offsetof(unit_datum, desired_weapon_index) == 0x100);
static_assert(offsetof(unit_datum, weapons) == 0x104);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

struct unit : object {
    unit_datum unit;
}; static_assert(sizeof(unit) == 0x4CC);

}

#endif // SENTINEL__STRUCTURES__UNIT_HPP
