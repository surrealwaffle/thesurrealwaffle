
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/globals.hpp>
#include "reve/console.hpp"
#include "reve/globals.hpp"

#include <cstring> // std::memcpy

#include <algorithm>   // std::min
#include <string_view> // std::string_view

SENTINEL_API
sentinel::globals_aggregate_type sentinel_Globals_globals = {/* ZERO INITIALIZED */};

SENTINEL_API
sentinel::table_aggregate_type sentinel_Globals_tables = {/* ZERO INITIALIZED */};

SENTINEL_API
sentinel::GameEdition
sentinel_Globals_Edition = sentinel::GameEdition::combat_evolved;

SENTINEL_API
bool
sentinel_Globals_GetCommandLineArg(const char* target, const char** value)
{
    const auto argc = reve::globals::ptr_CommandLineArgs->argc;
    const auto argv = reve::globals::ptr_CommandLineArgs->argv;

    if (value)
        *value = nullptr;

    if (!argc || !argv || !target)
        return false;

    int i = 0;
    for (; i < argc; ++i) {
        std::string_view arg = argv[i];
        if (arg[0] != '-')
            continue;

        if (arg == target)
            break;
    }

    if (i && i != argc) {
        if (++i != argc && argv[i][0] != '-' && value)
            *value = argv[i];
        return true;
    }

    return false;
}

/*
SENTINEL_API
sentinel::int32 sentinel__Globals_GetLocalPlayerNetworkIndex() {
    sentinel::int32 network_index = -1;
    for (sentinel::player const& player : *sentinel__Player_GetPlayers()) {
        if (player.local_index != -1) {
            network_index = player.network_index;
            break;
        }
    }

    return network_index;
}
*/
