#pragma once

#include <optional>
#include <string_view>

namespace simulacrum { namespace ai {

void reset();

bool load();

void recalculate_navigation(std::optional<std::string_view> cache_name);

void update(float seconds, long ticks);

} } // namespace simulacrum::ai
