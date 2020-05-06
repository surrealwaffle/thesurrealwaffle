#pragma once

#include <sentinel/fundamental_types.hpp>

namespace sentutil { namespace constants {

inline constexpr sentinel::real pi       = 0x1.921FB6p1;
inline constexpr sentinel::real half_pi  = pi / 2;
inline constexpr sentinel::real twice_pi = 2 * pi;

inline constexpr sentinel::real tick_interval      = 1.0f / 30.0f;
inline constexpr sentinel::real half_tick_interval = 1.0f / 60.0f;

} } // namespace sentutil::constants
