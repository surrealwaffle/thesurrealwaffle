
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

namespace sentutil { namespace effect {

template<class Filter>
bool install_effects_update_filter(Filter) { return false; }

} } // namespace sentutil::effect
