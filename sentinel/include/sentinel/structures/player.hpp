
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__STRUCTURES__PLAYER_HPP
#define SENTINEL__STRUCTURES__PLAYER_HPP

#include <cstddef> // offsetof

#include <sentinel/types.hpp>
#include <sentinel/structures/unit.hpp>

namespace sentinel {

struct player {
    identity_salt salt;
    index_short local_index; ///< The index of the player on this machine (`0` on PC), or `-1` if the player is remote.
    h_wchar name[12];

    int32 unknown00; // 0x01C

    index_long team; ///< red: `0`, blue: `1`, on FFA each player is assigned their own team

    // related to current usable item, cant be bothered to figure them out right now
    int32 unknown01; // 0x024
    int16 unknown02; // 0x028
    int16 unknown03; // 0x02A

    ticks_long respawn_time; ///< The number of ticks remaining until the player respawns.

    int32 unknown04; // 0x030

    identity<sentinel::unit> unit;      ///< The identity of the unit the player is currently controlling.
    identity<sentinel::unit> last_unit; ///< The identity of the unit the player previous controlled, excluding the current #unit.

    int32 unknown05[10]; // 0x03C

    char  unknown06[3];
    index_byte network_index;

    int32 unknown07[29]; // 0x068

    int32 ping; ///< The round-trip time between the player and the server, in milliseconds.

    int32 unknown08[72]; // 0x0E0

    constexpr bool is_local() const noexcept { return local_index != -1; }
    constexpr bool is_alive() const noexcept { return static_cast<bool>(unit); }
}; static_assert(sizeof(player) == 0x200);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(player, salt) == 0x000);
static_assert(offsetof(player, local_index) == 0x002);
static_assert(offsetof(player, name) == 0x004);
static_assert(offsetof(player, team) == 0x020);
static_assert(offsetof(player, respawn_time) == 0x02C);
static_assert(offsetof(player, unit) == 0x034);
static_assert(offsetof(player, last_unit) == 0x038);
static_assert(offsetof(player, network_index) == 0x067);
static_assert(offsetof(player, ping) == 0x0DC);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

}

#endif // SENTINEL__STRUCTURES__PLAYER_HPP
