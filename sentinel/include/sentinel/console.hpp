
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** \file console.hpp
 *
 * \brief Console and terminal functions and structures.
 */

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

/** \addtogroup exports
 * @{ */

extern "C" {

/** \brief Prints colored text to the console terminal.
 *
 * \param[in] color A pointer to the color to use, or `nullptr` for the default color.
 * \param[in] text The text to display.
 */
SENTINEL_API
void
sentinel_Console_TerminalPrint(const sentinel::argbf* color, sentinel::h_ccstr text);

/** \brief Processes \a expression as if it was entered into the console by the user.
 *
 * \param[in] expression The expression to evaluate.
 *
 * \return `true` on success, otherwise `false`.
 */
SENTINEL_API
bool
sentinel_Console_ProcessExpression(sentinel::h_ccstr expression);

/** Adds a command under \a name.
 *
 * These commands are supplied the space-separated arguments starting with the command name.
 *
 * \param[in] name The name of the command as exposed to the scripting system.
 * \param[in] data A pointer to the function instance data.
 * \param[in] call Executes the function, supplied by \a ptr, the number of arguments, and the arguments themselves.
 * \param[in] release Releases the function instance \a data. This may be `nullptr`, in which case this argument is ignored.
 * \return A handle to the command, which must be freed using \ref sentinel__FreeResource()
 *         or `nullptr` if the command could not be registered.
 */
/* TODO
SENTINEL_API
SENTINELHANDLE sentinel__Console_InstallCommand(char const* name,
                                                void* data,
                                                sentinel::boolean(*call)(void*, std::uint32_t, char const*[]),
                                                void(*release)(void*));
*/

} // extern "C"

/** @} */

