
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentutil/script.hpp>

namespace sentutil { namespace script { namespace impl {

std::unordered_map<int, evaluator_interface> registered_functions;

} } } // namespace sentutil::script::impl
