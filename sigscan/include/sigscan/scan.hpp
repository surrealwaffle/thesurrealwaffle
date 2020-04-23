
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // std::size_t

#include <functional>  // std::function
#include <optional>    // std::optional
#include <tuple>       // std::get, std::tuple_size
#include <type_traits> // std::integral_constant

#include "base.hpp"
#include "memory_range.hpp"

namespace sigscan {

/** \brief The type returned to the scanner when checking data against a pattern.
 */
enum ScanResult {
    scan_continue, ///< Indicates a partial match and that more data is required.
    scan_reject,   ///< Indicates a match failure.
    scan_accept,   ///< Indicates a match and consumes the last byte.
    scan_accept_noconsume ///< Indicates a match and ignores the last byte.
};

/** \brief Scans \a range for the first match of the scanner created by \a pattern.
 *
 * The scanner is created from \a pattern by the expression `pattern(first)`,
 * where `first` is a pointer to the first byte of an attempted match.
 *
 * A scanner is constructed at every point from which a match is attempted.
 * Bytes (of type \ref byte) are fed into the pattern in sequence.
 * Let `b` be the byte that is being matched against the pattern and `scanner`
 * be the scanner object created for the match as previously described.
 * If the result of `scanner(b)` is:
 *  * ScanResult::scan_continue then the pattern admits `b` but expects more data, or
 *  * ScanResult::scan_reject then the pattern rejects `b` and fails to match, or
 *  * ScanResult::scan_accept then the pattern has found a match and consumes `b`, or
 *  * ScanResult::scan_accept_noconsume then the pattern has found a match
 *                                      but does not consume `b`.
 *
 * If a scanner returns `scan_reject`, `scan_accept`, or `scan_accept_noconsume`,
 * then it will not be called again.
 *
 * \return The first \ref memory_range matching the scan, or
 *         `std::nullopt` if no match was found in \a range.
 */
template<class Pattern>
constexpr
std::optional<memory_range> scan_range(memory_range range, Pattern& pattern)
{
    for (byte* cursor = range.first; cursor != range.last; ++cursor) {
        auto scanner = pattern(cursor);
        for (auto match_cursor = cursor; match_cursor != range.last; ++match_cursor) {
            const byte b = *match_cursor;
            switch (ScanResult result = scanner(b)) {
            case scan_continue:         continue; // skips loop break below;
            case scan_reject:           break; // exits switch to break statement
            case scan_accept:           return memory_range{cursor, match_cursor + 1};
            case scan_accept_noconsume: return memory_range{cursor, match_cursor};
            }
            break; // skipped over in all cases above except scan_reject
        }
    }

    return std::nullopt;
}

/** \brief A scanner that contains the means to morph itself into a different scanner.
 *         This is used to achieve a sequence of scanners.
 *
 * When #scan holds no function, the scanner is said to be complete.
 * No more calls to #scan nor #next_scanner will be made after this point.
 *
 * This scanner respects ScanResult return semantics:
 *  * `scan_accept` becomes `scan_continue` if there is another scanner available;
 *  * `scan_accept_noconsume` is returned only for the last scanner in the sequence,
 *    otherwise the scans resulting in `scan_accept_noconsume` are collapsed.
 */
struct continuation_scanner
{
    byte* it;
    std::function<ScanResult(byte)>            scan;
    std::function<continuation_scanner(byte*)> next_scanner;

    ScanResult operator()(byte b)
    {
        ScanResult result = scan(b);
        while (result == scan_accept_noconsume && advance())
            result = scan(b);

        if (result != scan_accept_noconsume)
            ++it;

        if (result == scan_accept && advance())
            result = scan_continue;

        return result;
    }

private:
    bool advance()
    {
        *this = next_scanner(it);
        return static_cast<bool>(scan);
    }
};

/** \brief Constructs a \ref continuation_scanner from a tuple of \a patterns.
 *         The tuple \a patterns must outlive the resulting scanner.
 */
template<class Tuple>
continuation_scanner make_continuation_scanner(const Tuple& patterns, byte* first)
{
    auto make_scanner = [&patterns]
                        (auto& self, auto index, byte* first) -> continuation_scanner
    {
        if constexpr (index >= std::tuple_size<Tuple>()) {
            return {
                first,
                [] (byte) { return scan_accept_noconsume; },
                [] (byte* p) { return continuation_scanner{p, nullptr, nullptr}; }
            };
        } else {
            auto next_index = std::integral_constant<std::size_t, index + 1>{};
            auto& pattern = std::get<index>(patterns);
            return {
                first,
                pattern(first),
                [self, next_index] (byte* p) { return self(self, next_index, p); }
            };
        }
    };

    auto zero_index = std::integral_constant<std::size_t, 0>{};
    return make_scanner(make_scanner, zero_index, first);
}

} // namespace sigscan
