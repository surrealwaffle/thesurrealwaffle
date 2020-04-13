#pragma once

#include "types.hpp"
#include <sentinel/fwd/globals_fwd.hpp>

namespace sentinel {

struct terminal_type;

} // namespace sentinel

namespace reve { namespace console {

/** \brief The `printf` function type for Halo's terminal.
 *
 * A more accurate signature would be `void(const real* color, const char* fmt, ...)`
 * but this makes it difficult to interface with the function, as the `regparm`
 * attribute does not work with variadic functions.
 *
 * Instead, it is recommended to simply pass \a fmt as `"%s"` and pass a pre-formatted
 * string as \a arg. Interestingly, this is what Halo does anyway.
 */
using terminal_printf_tproc __attribute__((cdecl, regparm(1)))
    = void(*)(P_IN const real* color, /*EAX*/
              P_IN h_ccstr     fmt,   /*STACK*/
              P_IN h_ccstr     arg    /*STACK*/);

/** \brief Updates the terminal, decaying and removing expired entries.
 */
using terminal_update_tproc __attribute__((cdecl))
    = void(*)();

extern terminal_printf_tproc proc_TerminalPrintf;
extern terminal_update_tproc proc_TerminalUpdate;

//extern sentinel::terminal_type* ptr_Terminal;

inline sentinel::terminal_type*& ptr_Terminal = sentinel_Globals_globals.terminal;

/** \brief Replacement function for \ref proc_TerminalUpdate.
 *         Fixes the terminal to update per tick rather than per render frame.
 */
void hook_TerminalUpdate();

bool Init();

void Debug();

} } // namespace reve::console

