
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "console.hpp"

#include <sentinel/config.hpp>
#include <sentinel/structures/globals.hpp>
#include "globals.hpp"

namespace reve { namespace console {

terminal_printf_tproc proc_TerminalPrintf = nullptr;
terminal_update_tproc proc_TerminalUpdate = nullptr;

// sentinel::terminal_type* ptr_Terminal = nullptr;

void hook_TerminalUpdate()
{
    // Issue: proc_TerminalUpdate() is called on every render frame
    // The fix is to update by the number of ticks in the cumulative update
    // This is technically off by a cumulative update, as ticks_in_update does not
    // get set until after the terminal update.
    int ticks = (*reve::globals::ptr_pGameTimeGlobals)->ticks_in_update;
    while (--ticks >= 0)
        proc_TerminalUpdate();
}

bool Init()
{
    return proc_TerminalPrintf
        && proc_TerminalUpdate
        && ptr_Terminal;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_TerminalPrintf);
    SENTINEL_DEBUG_VAR("%p", proc_TerminalUpdate);
    SENTINEL_DEBUG_VAR("%p", ptr_Terminal);
}

} } // namespace reve::console
