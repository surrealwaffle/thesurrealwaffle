
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentutil/globals.hpp>

bool get_command_line(const char* target)
{
    return sentinel_Globals_GetCommandLineArg(target, nullptr);
}

bool get_command_line(const char* target, const char*& target_value)
{
    target_value = nullptr;
    return sentinel_Globals_GetCommandLineArg(target, &target_value);
}
