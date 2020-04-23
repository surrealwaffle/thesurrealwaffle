
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/script.hpp>
#include "reve/script.hpp"

#include <cstring> // memcpy

#include <algorithm> // std::min

SENTINEL_API
sentinel_handle
sentinel_script_InstallFunction(const sentinel::script_function* function,
                                int* function_index)
{
    return reve::script::InstallScriptFunction(function, function_index);
}

SENTINEL_API
sentinel::script_engine_functions_aggregate
sentinel_script_ScriptEngineFunctions = {/* ZERO INITIALIZED */};
