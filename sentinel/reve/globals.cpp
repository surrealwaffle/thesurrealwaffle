
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "globals.hpp"

#include <string_view>

#include <sentinel/config.hpp>

namespace reve { namespace globals {

/*
sentinel::game_time_globals_type**    ptr_pGameTimeGlobals    = nullptr;
sentinel::local_player_globals_type** ptr_pLocalPlayerGlobals = nullptr;
sentinel::tags_array_header_type**    ptr_pTagsArrayHeader    = nullptr;

sentinel::allocator_globals_type*     ptr_AllocatorGlobals    = nullptr;
sentinel::chat_globals_type*          ptr_ChatGlobals         = nullptr;
sentinel::console_globals_type*       ptr_ConsoleGlobals      = nullptr;
sentinel::machine_globals_type*       ptr_MachineGlobals      = nullptr;
sentinel::map_globals_type*           ptr_MapGlobals          = nullptr;
sentinel::camera_globals_type*        ptr_CameraGlobals       = nullptr;
sentinel::runtime_sound_globals_type* ptr_RuntimeSoundGlobals = nullptr;
*/

command_line_args_type* ptr_CommandLineArgs = nullptr;

h_ccstr ptr_EditionString = nullptr;

bool Init()
{
    if (!ptr_EditionString)
        return false;
    std::string_view edition_string(ptr_EditionString, 5);

    if (edition_string == "halor")
        edition = GameEdition::combat_evolved;
    else if (edition_string == "halom")
        edition = GameEdition::custom_edition;
    else
        return false;

    return ptr_pGameTimeGlobals
        && ptr_pLocalPlayerGlobals
        && ptr_pTagsArrayHeader
        && ptr_AllocatorGlobals
        && ptr_ChatGlobals
        && ptr_ConsoleGlobals
        && ptr_MachineGlobals
        && ptr_MapGlobals
        && ptr_CameraGlobals
        && ptr_RuntimeSoundGlobals
        && ptr_CommandLineArgs
        && ptr_MapFileHeader;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", ptr_EditionString);
    SENTINEL_DEBUG_VAR("%p", ptr_CommandLineArgs);
    SENTINEL_DEBUG_VAR("%p", ptr_pGameTimeGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_pLocalPlayerGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_pTagsArrayHeader);
    SENTINEL_DEBUG_VAR("%p", ptr_AllocatorGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_ChatGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_ConsoleGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_MachineGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_MapGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_CameraGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_RuntimeSoundGlobals);
    SENTINEL_DEBUG_VAR("%p", ptr_MapFileHeader);
}

} } // namespace reve::globals
