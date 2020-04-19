#pragma once

#include <sentinel/types.hpp>

namespace sentutil { namespace color {

inline constexpr sentinel::argbf default_console = {1.0f, 0.7f, 0.7f, 0.7f};

inline constexpr sentinel::argbf red = {1.0f, 1.0f, 0.0f, 0.0f};
inline constexpr sentinel::argbf green = {1.0f, 0.0f, 1.0f, 0.0f};
inline constexpr sentinel::argbf blue = {1.0f, 0.0f, 0.0f, 1.0f};

inline constexpr sentinel::argbf orange = {1.0f, 1.0f, 0.5f, 0.153f};
inline constexpr sentinel::argbf yellow = {1.0f, 1.0f, 1.0f, 0.0f};

} } // namespace sentutil::colors
