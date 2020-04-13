#include <sentinel/raycast.hpp>

#include "reve/raycast.hpp"

SENTINEL_API
bool
sentinel_Raycast_Test(sentinel::mask_long            mask,
                      sentinel::position3d const*    source,
                      sentinel::position3d const*    delta,
                      sentinel::identity<sentinel::object_table_datum> object_id,
                      sentinel::raycast_result_type* result)
{
    return reve::raycast::proc_CastRay(mask,
                                       source->data(),
                                       delta->data(),
                                       object_id.raw,
                                       result);
}
