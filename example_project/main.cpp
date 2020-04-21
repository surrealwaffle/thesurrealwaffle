
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "main.h"

#include <sentutil/all.hpp>

#include <cwchar>

#include <algorithm>
#include <chrono>
#include <optional>
#include <string_view>

/**
 * This sample project install a new console command: chat_all, which puts a supplied
 * string into all chat. If a string is not supplied, it defaults to "Hello, World!".
 * Additionally, it returns the integer value `314`.
 *
 * This allows the user (with devmode enabled) to enter the expression
 * `begin (sleep (chat_say "Hi!")) (chat_say "Pirate here.")`
 * into the console, and it will say "Hi!" and "Pirate here." in all chat, roughly
 * 10 seconds apart.
 *
 * Any installations through the `sentinel` facilities return a `sentinel_handle`
 * which represents the lifetime of that installation, and the installation is undone
 * by calling `sentinel_FreeHandle` on the returned handle.
 *
 * However, installations performed through `sentutil` are managed with global
 * lifetime, but can be uninstalled all at once via `handle::clear_managed_handles()`.
 * When the application closes, the handles are freed automatically.
 */

namespace {

long chat_say(std::optional<std::string_view> text)
{
    sentutil::chat::send_chat(text.value_or("Hello, World!"));
    return 314L;
}

bool Load() {
    // When given a function pointer, install_script_function will deduce the return
    // type and arguments and create the necessary parser and evaluation coroutine.
    //
    // Additionally, one may also pass the function as a std::function, which allows
    // for lambdas to be integrated into Halo's scripting engine.
    return sentutil::script::install_script_function<"chat_say">(
            chat_say,
            "sends chat over the global channel", // help text (usage) - optional
            "[<string: message>]"                 // help text (parameters) - optional
            );
}

} // namespace (anonymous)

extern "C" DLL_EXPORT BOOL APIENTRY DllMain([[maybe_unused]] HINSTANCE hinstDLL,
                                            DWORD fdwReason,
                                            [[maybe_unused]]LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            if (!Load())
                return FALSE;
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}
