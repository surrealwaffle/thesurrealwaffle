#pragma once

namespace sentutil { namespace effect {

template<class Filter>
bool install_effects_update_filter(Filter) { return false; }

} } // namespace sentutil::effect
