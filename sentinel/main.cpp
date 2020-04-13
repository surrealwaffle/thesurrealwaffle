#include <wunduws.hpp>

#include <cstdio>
#include <algorithm> // std::search

#include <detours/detours.hpp>
#include "reve/descriptors.hpp"

#include <sentinel/all.hpp>

namespace {

/** \brief Attempts to load `sentinel` into the process.
 *
 * \return `TRUE` on if `sentinel` loaded successfully, otherwise `FALSE`.
 */
BOOL AttachSentinelLibrary(HINSTANCE hinstDLL);

/** \brief Unloads `sentinel` from the process.
 *
 * \return `TRUE` if sentinel unloaded successfully, otherwise `FALSE`.
 */
BOOL DetachSentinelLibrary(HINSTANCE hinstDLL);

/** \brief Searches the command line for \a str.
 *
 * \return A pointer to the first occurrence of \a str in the command line, or
 *         `nullptr` if \a str could not be found.
 */
LPCTSTR SearchCommandLine(LPCTSTR str);

bool is_attached        = false; ///< Indicates a successful load/attach.
bool is_console_created = false; ///< Indicates a console was created by `sentinel`.

}

extern "C"
BOOL
APIENTRY DllMain(HINSTANCE hinstDLL,
                 DWORD fdwReason,
                 [[maybe_unused]] LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH: return AttachSentinelLibrary(hinstDLL);
        case DLL_PROCESS_DETACH: return DetachSentinelLibrary(hinstDLL);
        case DLL_THREAD_ATTACH: break;
        case DLL_THREAD_DETACH: break;
    }
    return TRUE; // successful
}

namespace {

BOOL AttachSentinelLibrary(HINSTANCE hinstDLL) {
    DisableThreadLibraryCalls(hinstDLL);

    // switch -no-sentinel suppresses sentinel
    if (SearchCommandLine(TEXT("-no-sentinel")))
        return TRUE;

    // switch -create-console allocates a console for Halo to show stdout
    if (SearchCommandLine(TEXT("-create-console")) && AllocConsole()) {
        freopen("CON", "w", stdout);
        is_console_created = true;
    }

    // pin the module, so it does not reload
    HMODULE hModule;
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS
                           | GET_MODULE_HANDLE_EX_FLAG_PIN,
                           (LPCTSTR)hinstDLL,
                           &hModule))
        return FALSE;
    FreeLibrary(hModule);

    if (!reve::Init()) {
        detours::management::clear_managed_patches();
        return FALSE;
    }

    is_attached = true;

    return TRUE;
}

BOOL DetachSentinelLibrary([[maybe_unused]] HINSTANCE hinstDLL)
{
    if (is_console_created)
        FreeConsole();

    if (is_attached)
        detours::management::clear_managed_patches();

    return TRUE;
}

LPCTSTR SearchCommandLine(LPCTSTR str)
{
    if (!str)
        return nullptr;

    LPTSTR cmd_line = GetCommandLine();
    if(!cmd_line)
        return nullptr;

    LPTSTR cmd_line_end = cmd_line;
    for (; *cmd_line_end; ++cmd_line_end) { /* SEEK TO NULL TERMINATOR */ }

    LPCTSTR str_end = str;
    for (; *str_end; ++str_end) { /* SEEK TO NULL TERMINATOR */ }

    LPCTSTR it = std::search(cmd_line, cmd_line_end, str, str_end);
    return it != cmd_line_end ? it : nullptr;
}

}
