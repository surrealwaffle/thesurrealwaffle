
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <functional> // std::function
#include <optional>   // std::optioonal
#include <vector>     // std::vector

#include "base.hpp"

namespace sigscan {

/** \brief Represents a contiguous range of memory.
 */
struct memory_range {
    byte* first; ///< A pointer to the first byte of the range.
    byte* last;  ///< A pointer to the last (1-past-end) byte of the range.
};

/** \brief Enumerates the text segments of the module by \a module_name,
 *         or the module of the starting process if \a module_name is `nullptr`.
 *
 * \return The text segments of the module,
 *         or `std::nullopt` on error.
 */
std::optional<std::vector<memory_range>> get_text_segments(const char* module_name);

/** \brief Enumerates the text segments of the module of the starting process.
 *
 * \return The text segments of the module,
 *         or `std::nullopt`.
 */
inline std::optional<std::vector<memory_range>> get_text_segments()
    { return get_text_segments(nullptr); }

/** \brief Changes the protection on a region of pages to make them
 *         readable, writable, and executable.
 *
 * \return A scope guard that restores the protection upon leaving scope.
 */
std::optional<scope_guard<std::function<void()>>> hold_range_rwx(memory_range range);

/** \brief Flushes the instruction cache for the current process in the \a range.
 */
void flush_range(memory_range range);

} // namespace sigscan
