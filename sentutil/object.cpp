
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentutil/object.hpp>

#include <utility> // std::inplace

namespace sentutil { namespace object { namespace impl {

int get_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                const char* marker_name,
                sentinel::object_marker_result_type* result,
                sentinel::h_short result_count)
{ return sentinel_Object_GetMarkers(object, marker_name, result, result_count); }

} } } // namespace sentutil::object::impl

namespace sentutil { namespace object {

std::optional<sentinel::object_marker_result_type>
get_object_marker(const sentinel::identity<sentinel::object_table_datum>& object,
                  const char* marker_name)
{
    std::optional<sentinel::object_marker_result_type> result(std::in_place);
    if (impl::get_markers(object, marker_name, &result.value(), 1) == 0)
        result = std::nullopt;

    return result;
}

sentinel::point3d get_unit_camera(const sentinel::identity<sentinel::unit>& unit)
{
    sentinel::point3d position;
    sentinel_Unit_GetCameraPosition(unit, &position);
    return position;
}

sentinel::real3d extrapolate_unit_delta(const sentinel::identity<sentinel::unit>& unit,
                                        const sentinel::real seconds)
{
    sentinel::real3d delta = {};
    sentinel::real3d unk1 = {};
    sentinel::real3d unk2 = {};

    sentinel_Engine_ExtrapolateLocalUnitDelta(&delta,
                                              &unk1, &unk2,
                                              seconds);

    return delta;
}

} } // namespace sentuti::object
