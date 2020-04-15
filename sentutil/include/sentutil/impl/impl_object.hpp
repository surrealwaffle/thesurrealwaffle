#pragma once

#include <sentinel/object.hpp>

namespace sentutil { namespace object { namespace impl {

int get_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                const char* marker_name,
                sentinel::object_marker_result_type* result,
                sentinel::h_short result_count);

} } } // namespace sentutil::object::impl
