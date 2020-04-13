/** \file fundamental_types.hpp
 * \brief Provides definitions for the essential data types used to interface with the Halo client.
 */

#pragma once

#include <climits> // CHAR_BIT
#include <cstdint> // std::int8_t, std::int16_t, std::int32_t, std::int64_t,
                   // std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t

#include <limits>      // std::numeric_limits
#include <type_traits>

namespace sentinel {

namespace fundamental_types {

// ---------------------------------
// Specific-width integers

using int8  = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;

using uint8  = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

// ---------------------------------
// Definitions for byte, short, long
// Also char and wide char

using h_byte  = char;
using h_short = std::int16_t;
using h_long  = std::int32_t;;

using h_ubyte  = unsigned char;
using h_ushort = std::uint16_t;
using h_ulong  = std::uint32_t;

using h_char  = char;
using h_wchar = wchar_t;

using h_cstr   = char*;
using h_ccstr  = const char*;
using h_wcstr  = wchar_t*;
using h_cwcstr = const wchar_t*;

// ---------------------------------
// Miscellaneous

using boolean = bool;
using real    = float;

using bool8  = std::int8_t;
using bool16 = std::int16_t;
using bool32 = std::int32_t;

// ------------------------
// Semantic types

using index_byte  = std::int8_t;
using index_short = std::int16_t;
using index_long  = std::int32_t;

using index_ubyte  = std::uint8_t;
using index_ushort = std::uint16_t;
using index_ulong  = std::uint32_t;

using ticks_byte  = std::int8_t;
using ticks_short = std::int16_t;
using ticks_long  = std::int32_t;

using ticks_ubyte  = std::uint8_t;
using ticks_ushort = std::uint16_t;
using ticks_ulong  = std::uint32_t;

using frames_ubyte  = std::uint8_t;
using frames_ushort = std::uint16_t;
using frames_ulong  = std::uint32_t;

using flags_byte  = std::uint8_t;
using flags_short = std::uint16_t;
using flags_long  = std::uint32_t;

using mask_byte  = std::uint8_t;
using mask_short = std::uint16_t;
using mask_long  = std::uint32_t;

using enum_byte  = std::int8_t;
using enum_short = std::int16_t;
using enum_long  = std::int32_t;

using salt_byte  = std::uint8_t;
using salt_short = std::uint16_t;
using salt_long  = std::uint32_t;

using identity_raw   = std::uint32_t;
using identity_salt  = salt_short;
using identity_index = index_short;

using channel = std::int32_t;

using radians = real;

using fraction = real;        ///< A real value in the closed interval `[0, 1]`.
using signed_fraction = real; ///< A real value in the closed interval `[-1, 1]`.

/** \brief Values of this type are typically used to mark tag types.
 *         In Halo's source code, this is probably a multi-character constant.
 */
using signature = uint32_t;

// ---------------------------------
// Assertions

static_assert(CHAR_BIT == 8, "each byte must be an octet");
static_assert(sizeof(boolean) == 1, "boolean must be a single byte wide");
static_assert(sizeof(real) == 4 && std::numeric_limits<real>::is_iec559,
              "real must be single-precision IEEE 754");
static_assert(sizeof(h_char) == 1); // superfluous
static_assert(sizeof(h_wchar) == 2);

} // namespace (sentinel::)fundamental_types

using namespace sentinel::fundamental_types;

} // namespace sentinel

