
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>
#include <sentinel/console.hpp>

namespace sentutil{ namespace console {

/** \brief Prints colored, formatted text to the console.
 *
 * \param[in] color The color to use.
 * \param[in] fmt   The format string, as in `std::printf`.
 */
void cprintf(const sentinel::argbf& color,
             char const* fmt, ...) SENTINEL_ATTRIBUTES(format(printf, 2, 3));

/** \brief Prints formatted text to the console in the default color.
 *
 * \param[in] fmt The format string, as in `std::printf`.
 */
void cprintf(char const* fmt, ...) SENTINEL_ATTRIBUTES(format(printf, 1, 2));

/** \brief Prints colored text to the console.
 *
 * \param[in] color The color to use.
 * \param[in] text  The text to print.
 */
inline void cprint(const sentinel::argbf& color,
                   const char* text) { cprintf(color, "%s", text); }

/** \brief Prints text to the console in the default color.
 *
 * \param[in] text  The text to print.
 */
inline void cprint(const char* text) { cprintf("%s", text); }

/** \brief Submits \a expression to the script engine as if from the console.
 */
void process_expression(const char* expression);

} } // namespace sentutil::console
