/** \file globals.hpp
 *
 * \brief Halo globals and miscellaneous values.
 */

#pragma once

#include <type_traits>

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

#include <sentinel/structures/console.hpp>
#include <sentinel/structures/globals.hpp>
#include <sentinel/structures/table.hpp>
#include <sentinel/structures/tag.hpp>

/** \addtogroup exports
 * @{
 */

namespace sentinel {

} // namespace sentinel

extern "C" {

extern
SENTINEL_API
sentinel::globals_aggregate_type sentinel_Globals_globals;

extern
SENTINEL_API
sentinel::table_aggregate_type sentinel_Globals_tables;

extern
SENTINEL_API
sentinel::GameEdition
sentinel_Globals_Edition;

/** \brief Checks for the presence of a command line argument by the name \a arg
 *         and outputs the value passed to that argument through \a value.
 *
 * This function replicates the behaviour of Halo's subroutine that performs the
 * same function.
 *
 * If \a value is `nullptr`, then it is never dereferenced.
 * Otherwise, `*value` is assigned the argument proceeding \a arg if:
 *  * the proceeding argument exists (does not exceed the bounds of `argv`);
 *  * the proceeding arugment does not begin with `'-'`.
 * If any of these conditions are not met, then `*value` is assigned `nullptr`.
 *
 * \param[in] target The parameter target (must begin with a `'-'`).
 * \param[out] value The value given to the target parameter.
 *
 * \return `true` if an argument by \a arg exists, otherwise `false`.
 */
SENTINEL_API
bool
sentinel_Globals_GetCommandLineArg(const char* target, const char** value);

} // extern "C"

/** @} */
