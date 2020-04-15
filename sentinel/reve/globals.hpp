#pragma once

#include "types.hpp"
#include <sentinel/fwd/globals_fwd.hpp>

namespace sentinel {

struct game_time_globals_type;
struct machine_globals_type;
struct allocator_globals_type;
struct camera_globals_type;
struct chat_globals_type;
struct console_globals_type;
struct map_globals_type;
struct local_player_globals_type;
struct runtime_sound_globals_type;
struct tags_array_header_type;

} // namespace sentinel

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

extern command_line_args_type* ptr_CommandLineArgs;

extern h_ccstr ptr_EditionString;

bool Init();

void Debug();

} } // namespace reve::globals
