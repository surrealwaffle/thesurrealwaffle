
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/fwd/globals_fwd.hpp>
#include <sentinel/structures/table.hpp>
#include <sentinel/tags/scenario.hpp>
#include <sentinel/tags/structure_bsp.hpp>
#include <sentinel/tags/collision_bsp.hpp>
#include <sentinel/tags/game_globals.hpp>

namespace sentinel {

struct game_time_globals_type {
    boolean is_ready;
    boolean is_running;
    boolean is_paused;

    int32 unknown[2];

    ticks_long  game_ticks; // updates each processed tick
    ticks_short ticks_in_update; // only set after cumulative update is performed
    ticks_long  game_ticks2; // seems to be the same as game_ticks
    real        game_speed;
    real        seconds_since_tick; // always partial, never more than a tick interval
}; static_assert(sizeof(game_time_globals_type) == 0x20);

struct machine_globals_type {
    int64 now_count;  // LARGE_INTEGER of time in frequency counts (see QueryPerformanceCounter)
    int32 now_millis; // LARGE_INTEGER of time in milliseconds
    int64 unknown_counter; // taken from QueryPerformanceCounter, not sure what its about

    boolean unknown01;

    real  frame_dt; // in seconds, up to 1 second

    // not sure if this is the same structure
    enum_short connection_type; // 3:pre-load, 2:hosting, 1:client, 0:SP/UI
    uint16 unknown02;
    uint32 unknown03;
}; static_assert(sizeof(machine_globals_type) == 0x28);

struct allocator_globals_type {
    h_byte* base;  // 0x00
    h_ulong allocated; ///< `base + allocated` is a pointer to the first free byte

    int32 unused00; // 0x08

    h_ulong allocator_hash; // 0x0C

    boolean unknown00; // 0x10
    boolean unknown01; // 0x11

    // 0x14
    ticks_long unknown02; // written with value Globals::GameTimeGlobals.game_ticks
                          // also checked when evaluating (reverted) against
                          // Globals::GameTimeGlobals.game_ticks
    h_byte* unknown03; // 0x18 - only written to when the allocator is initialized
    h_ulong allocator_handle; // 0x1C - returned from GlobalAlloc

    boolean unknown04; // 0x20

    h_byte* buffer; // 0x24 - points to a buffer for reading from a file/device
    h_byte* unknown05; // 0x28 - only written to when allocator is initialized
    boolean unknown06; // 0x2c
    boolean unknown07; // 0x2d

    h_ulong handle_savegame_bin; // 0x30 - returned from CreateFileA

    char savegame_bin_path[0x100]; // 0x34
    char core_path[0x100]; // 0x134

    h_ulong event_handle; // 0x234 - created from CreateEventA

    boolean unknown08; // 0x238
    boolean unknown09; // 0x239

    h_byte* data_begin() const { return base; }
    h_byte* data_end() const { return base + allocated; }

    const h_byte* data_cbegin() const { return base; }
    const h_byte* data_cend() const { return base + allocated; }
}; static_assert(sizeof(allocator_globals_type) == 0x23C);

struct camera_globals_type {
    position3d  position;
    real        unknown[5];
    direction3d forward;
    direction3d up;
}; // incomplete, size is much larger (29C on PC)

struct chat_globals_type {
    boolean   is_open; ///< `true` if the chat control is open and focused, otherwise `false`.
    enum_long channel; ///< 0 - all, 1 - team, 2 - vehicle, `-1` - none.

    // some things here are explicitly initialized to specific values
    // i cant find any references about writing
    h_byte unknown[0x1C0];

    // need to find out what timer is used for the start time, but
    // the expire time is set to 8 seconds
    // measured in milliseconds
    uint32 entry_expire_times[8];
}; static_assert(sizeof(chat_globals_type) == 0x1E8);

struct local_player_globals_type {
    struct player_info_type {
        identity<unit> unit;      // 0x00

        int32 unknown01[2]; // 0x04, something to do with actions

        real yaw;           // 0x0C
        real pitch;         // 0x10

        direction2d movement; // 0x14
        real primary_trigger; // 0x1C
        int16 desired_weapon_index;  // 0x20
        int16 desired_grenade_index; // indexes into bytes at +0x3E
        int16 desired_zoom_level;   // 0x24

        h_byte unknown02; // 0x26
        h_byte unknown03; // 0x27
        int32  unknown04[2]; // 0x28
        real   unknown05; // 0x30
        int32  unknown06[3]; // 0x34
    }; static_assert(sizeof(player_info_type) == 0x40);

    int32 unknown00[4];
    player_info_type players[1];
}; static_assert(sizeof(local_player_globals_type) == 0x50);

struct map_globals_type {
    ticks_long          frames_rendered; ///< The number of render frames since map load.
    tags::scenario      *scenario;       ///< A pointer to the scenario tag definition.
    tags::collision_bsp *collision_bsp;  ///< A pointer to the current collision BSP tag.
    void* unknown0;
    tags::collision_bsp *collision_bsp1; ///< A pointer to the current collision BSP tag.
    tags::structure_bsp *structure_bsp;  ///< A pointer to the current structure BSP tag.
    tags::game_globals  *game_globals;   ///< A pointer to the map's game globals.
    int8  unknown1;
    int32 unknown2;
}; static_assert(sizeof(map_globals_type) == 0x24);

struct runtime_sound_globals_type {
	fraction current_volume; ///< Controls the overall volume and interpolates to either `0` or `1`, regardless of #master_volume.
	fraction music_volume;   ///< The volume level of music.
	fraction master_volume;  ///< The overall volume level.
	fraction effects_volume; ///< The volume level of effects.
	int16 unknown0;          ///< 0x10, has something to do with the number of sounds/sound variety
	boolean  no_sound;       ///< Set to `true` to disable sound and `false` to enable sound.
	int8  unknown1;          ///< 0x13
	int16 variety;           ///< 0=low, 1=medium, 2=height, see #unknown0
	int16 PAD0;              ///< 0x16, unused
	int8  unknown2;          ///< 0x18
	table_type<void> *lpSounds; ///< Pointer to the sounds table.
}; static_assert(sizeof(runtime_sound_globals_type) == 0x20);

struct map_file_header_type {
    signature head;

    uint32 client_edition; // 0x7 on PC, 0x261 on CE
    int32  decompressed_file_size;
    uint32 unknown00;
    int32  meta_data_offset;
    int32  meta_data_size;
    uint32 unknown01;
    uint32 unknown02;
    h_char map_name[32];
    h_char client_version[32];
    enum_short map_type; // 0=SP, 1=MP, 2=UI
    uint32 checksum;

    char pad[0x794];

    signature foot;
}; static_assert(sizeof(map_file_header_type) == 0x800);

struct map_cache_context {
    boolean              is_loaded;
    map_file_header_type map_file_header;
}; static_assert(sizeof(map_cache_context) == 0x804);

struct profile_user_name_type {
    h_wchar tab;
    h_wchar name[12];
};

static_assert(offsetof(machine_globals_type, now_count) == 0x00);
static_assert(offsetof(machine_globals_type, now_millis) == 0x08);
static_assert(offsetof(machine_globals_type, unknown_counter) == 0x10);
static_assert(offsetof(machine_globals_type, frame_dt) == 0x1C);
static_assert(offsetof(machine_globals_type, connection_type) == 0x20);

static_assert(offsetof(chat_globals_type, is_open) == 0x00);
static_assert(offsetof(chat_globals_type, channel) == 0x04);
static_assert(offsetof(chat_globals_type, entry_expire_times) == 0x1C8);

static_assert(offsetof(local_player_globals_type::player_info_type, unit)                  == 0x00);
static_assert(offsetof(local_player_globals_type::player_info_type, yaw)                   == 0x0C);
static_assert(offsetof(local_player_globals_type::player_info_type, pitch)                 == 0x10);
static_assert(offsetof(local_player_globals_type::player_info_type, movement)              == 0x14);
static_assert(offsetof(local_player_globals_type::player_info_type, primary_trigger)       == 0x1C);
static_assert(offsetof(local_player_globals_type::player_info_type, desired_weapon_index)  == 0x20);
static_assert(offsetof(local_player_globals_type::player_info_type, desired_grenade_index) == 0x22);
static_assert(offsetof(local_player_globals_type::player_info_type, desired_zoom_level)    == 0x24);
static_assert(offsetof(local_player_globals_type::player_info_type, unknown05)             == 0x30);

static_assert(offsetof(map_globals_type, frames_rendered) == 0x00);
static_assert(offsetof(map_globals_type, scenario)        == 0x04);
static_assert(offsetof(map_globals_type, collision_bsp)   == 0x08);
static_assert(offsetof(map_globals_type, collision_bsp1)  == 0x10);
static_assert(offsetof(map_globals_type, structure_bsp)   == 0x14);
static_assert(offsetof(map_globals_type, game_globals)    == 0x18);

static_assert(offsetof(camera_globals_type, position) == 0x00);
static_assert(offsetof(camera_globals_type, forward)  == 0x20);
static_assert(offsetof(camera_globals_type, up)       == 0x2C);

static_assert(offsetof(runtime_sound_globals_type, current_volume) == 0x00);
static_assert(offsetof(runtime_sound_globals_type, music_volume) == 0x04);
static_assert(offsetof(runtime_sound_globals_type, master_volume) == 0x08);
static_assert(offsetof(runtime_sound_globals_type, effects_volume) == 0x0C);
static_assert(offsetof(runtime_sound_globals_type, unknown0) == 0x10);
static_assert(offsetof(runtime_sound_globals_type, no_sound) == 0x12);
static_assert(offsetof(runtime_sound_globals_type, unknown1) == 0x13);
static_assert(offsetof(runtime_sound_globals_type, variety) == 0x14);
static_assert(offsetof(runtime_sound_globals_type, unknown2) == 0x18);
static_assert(offsetof(runtime_sound_globals_type, lpSounds) == 0x1C);

static_assert(offsetof(map_file_header_type, head) == 0x000);
static_assert(offsetof(map_file_header_type, client_edition) == 0x004);
static_assert(offsetof(map_file_header_type, decompressed_file_size) == 0x008);
static_assert(offsetof(map_file_header_type, meta_data_offset) == 0x010);
static_assert(offsetof(map_file_header_type, meta_data_size) == 0x014);
static_assert(offsetof(map_file_header_type, map_name) == 0x020);
static_assert(offsetof(map_file_header_type, client_version) == 0x040);
static_assert(offsetof(map_file_header_type, map_type) == 0x060);
static_assert(offsetof(map_file_header_type, checksum) == 0x064);
static_assert(offsetof(map_file_header_type, foot) == 0x7FC);

}

