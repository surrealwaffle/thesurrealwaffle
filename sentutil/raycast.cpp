#include <sentutil/raycast.hpp>

#include <utility> // std::inplace

namespace sentutil { namespace raycast {

std::optional<sentinel::raycast_result_type>
cast_ray(sentinel::mask_long      mask,
         const sentinel::point3d& source,
         const sentinel::point3d& delta,
         const identity<object_table_datum>& exclude)
{
    std::optional<sentinel::raycast_result_type> result(std::in_place);
    if (!sentinel_Raycast_Test(mask, &source, &delta, exclude, &result.value()))
        result = std::nullopt;
    return result;
}

} }
