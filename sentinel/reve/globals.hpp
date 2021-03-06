
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "types.hpp"
#include <sentinel/fwd/globals_fwd.hpp>

namespace reve {

using sentinel::GameEdition;

inline sentinel::GameEdition& edition = sentinel_Globals_Edition;

} // namespace reve

namespace reve { namespace globals {

struct command_line_args_type {
    h_ccstr* argv; ///< The argument values.
    h_long   argc; ///< The number of arguments passed through #argv.
};

/*
extern sentinel::game_time_globals_type**    ptr_pGameTimeGlobals;
extern sentinel::local_player_globals_type** ptr_pLocalPlayerGlobals;
extern sentinel::tags_array_header_type**    ptr_pTagsArrayHeader;

extern sentinel::allocator_globals_type*     ptr_AllocatorGlobals;
extern sentinel::camera_globals_type*        ptr_CameraGlobals;
extern sentinel::chat_globals_type*          ptr_ChatGlobals;
extern sentinel::console_globals_type*       ptr_ConsoleGlobals;
extern sentinel::machine_globals_type*       ptr_MachineGlobals;
extern sentinel::map_globals_type*           ptr_MapGlobals;
extern sentinel::runtime_sound_globals_type* ptr_RuntimeSoundGlobals;
*/
inline sentinel::game_time_globals_type**&    ptr_pGameTimeGlobals = sentinel_Globals_globals.game_time_globals;
inline sentinel::local_player_globals_type**& ptr_pLocalPlayerGlobals = sentinel_Globals_globals.local_player_globals;
inline sentinel::tags_array_header_type**&    ptr_pTagsArrayHeader = sentinel_Globals_globals.tags_array_header;

inline sentinel::allocator_globals_type*&     ptr_AllocatorGlobals = sentinel_Globals_globals.allocator_globals;
inline sentinel::camera_globals_type*&        ptr_CameraGlobals = sentinel_Globals_globals.camera_globals;
inline sentinel::chat_globals_type*&          ptr_ChatGlobals = sentinel_Globals_globals.chat_globals;
inline sentinel::console_globals_type*&       ptr_ConsoleGlobals = sentinel_Globals_globals.console_globals;
inline sentinel::machine_globals_type*&       ptr_MachineGlobals = sentinel_Globals_globals.machine_globals;
inline sentinel::map_globals_type*&           ptr_MapGlobals = sentinel_Globals_globals.map_globals;
inline sentinel::runtime_sound_globals_type*& ptr_RuntimeSoundGlobals = sentinel_Globals_globals.runtime_sound_globals;
inline sentinel::map_file_header_type*&       ptr_MapFileHeader = sentinel_Globals_globals.map_file_header;
inline sentinel::profile_user_name_type*&     ptr_ProfileUserName = sentinel_Globals_globals.profile_user_name;
inline sentinel::map_cache_context*&          ptr_MapCacheContext = sentinel_Globals_globals.map_cache_context;

extern command_line_args_type* ptr_CommandLineArgs;

extern h_ccstr ptr_EditionString;

bool Init();

void Debug();

} } // namespace reve::globals
