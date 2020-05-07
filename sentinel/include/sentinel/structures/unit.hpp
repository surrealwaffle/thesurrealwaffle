
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

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
    ticks_long  unknown07[4];          // Related to time and pulling out a weapon.
    identity<>  unknown08;
    index_byte  current_grenade_index; ///< The type of grenade the unit has selected.
    index_byte  desired_grenade_index; ///< The type of grenade the unit will have selected when ready.
    h_byte      grenade_counts[2];

    index_byte current_zoom_level; ///< -1 for unzoomed, 0 = first zoom level, 1 = second zoom level, ...
    index_byte desired_zoom_level;

    int32 unknown09[106];
}; static_assert(sizeof(unit_datum) == 0x2D8);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(unit_datum, body_forward) == 0x30);
static_assert(offsetof(unit_datum, aim_forward) == 0x3C);
static_assert(offsetof(unit_datum, movement) == 0x84);
static_assert(offsetof(unit_datum, seat_index) == 0xFC);
static_assert(offsetof(unit_datum, weapon_index) == 0xFE);
static_assert(offsetof(unit_datum, desired_weapon_index) == 0x100);
static_assert(offsetof(unit_datum, weapons) == 0x104);
static_assert(offsetof(unit_datum, current_grenade_index) == 0x128);
static_assert(offsetof(unit_datum, desired_grenade_index) == 0x129);
static_assert(offsetof(unit_datum, grenade_counts) == 0x12A);
static_assert(offsetof(unit_datum, current_zoom_level) == 0x12C);
static_assert(offsetof(unit_datum, desired_zoom_level) == 0x12D);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

struct unit : object {
    unit_datum unit;

    identity<weapon> get_weapon() const { return unit.weapon_index == -1 ? invalid_identity : unit.weapons[unit.weapon_index]; }
}; static_assert(sizeof(unit) == 0x4CC);

} // namespace sentinel
