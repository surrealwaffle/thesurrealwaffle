
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

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
