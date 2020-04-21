
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/object.hpp>

namespace sentutil { namespace object { namespace impl {

int get_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                const char* marker_name,
                sentinel::object_marker_result_type* result,
                sentinel::h_short result_count);

} } } // namespace sentutil::object::impl
