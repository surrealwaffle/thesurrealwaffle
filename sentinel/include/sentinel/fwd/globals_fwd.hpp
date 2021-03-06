
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

namespace sentinel {

struct game_time_globals_type;
struct machine_globals_type;
struct allocator_globals_type;
struct camera_globals_type;
struct chat_globals_type;
struct console_globals_type;
struct local_player_globals_type;
struct map_globals_type;
struct runtime_sound_globals_type;
struct tags_array_header_type;
struct terminal_type;
struct map_cache_context;
struct map_file_header_type;
struct profile_user_name_type;

enum class GameEdition : int32 {
    combat_evolved,
    custom_edition
};

struct globals_aggregate_type {
    game_time_globals_type**    game_time_globals;
    local_player_globals_type** local_player_globals;
    tags_array_header_type**    tags_array_header;
    allocator_globals_type*     allocator_globals;
    camera_globals_type*        camera_globals;
    chat_globals_type*          chat_globals;
    console_globals_type*       console_globals;
    machine_globals_type*       machine_globals;
    map_globals_type*           map_globals;
    runtime_sound_globals_type* runtime_sound_globals;

    terminal_type*              terminal;

    map_file_header_type*       map_file_header;
    profile_user_name_type*     profile_user_name;

    map_cache_context*          map_cache_context;
};

} // namespace sentinel

extern "C" {

extern
SENTINEL_API
sentinel::globals_aggregate_type sentinel_Globals_globals;

extern
SENTINEL_API
sentinel::GameEdition
sentinel_Globals_Edition;

} // extern "C""
