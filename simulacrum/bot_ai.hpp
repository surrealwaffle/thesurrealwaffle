
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <optional>
#include <string_view>

namespace simulacrum { namespace ai {

void reset();

bool load();

void recalculate_navigation(std::optional<std::string_view> cache_name);

void update(float seconds, long ticks);

} } // namespace simulacrum::ai
