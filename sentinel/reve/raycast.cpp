#include "raycast.hpp"

#include <cstdio>

#include <sentinel/config.hpp>

namespace reve { namespace raycast {

cast_ray_tproc proc_CastRay = nullptr;

bool Init()
{
    return proc_CastRay;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_CastRay);
}

} } // namespace reve::raycast
