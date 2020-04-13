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
