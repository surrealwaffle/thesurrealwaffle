
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "object.hpp"

#include <sentinel/config.hpp>

namespace reve { namespace object {

get_object_markers_tproc       proc_GetObjectMarkers      = nullptr;
get_unit_camera_position_tproc proc_GetUnitCameraPosition = nullptr;

object_prototype_type** ptr_pObjectPrototypes = nullptr;

void GetUnitCameraPosition(identity_raw identity, real* out)
{
    // EAX, ECX, EDX get clobbered (proc_GetObjectCamera is cdecl)
    // EAX and EDX can be listed in clobbers, but ECX cannot
    // therefore we list ECX in the output operands and discard the result
    // in order to avoid discarding the asm statement, it must be marked volatile
    regint discard;
    asm volatile("call *%4 \n\t"
        : "=c" (discard), "=m" (out[0]), "=m" (out[1]), "=m" (out[2])
        : "rm" (proc_GetUnitCameraPosition), "c" (identity), "D" (out)
        : "cc", "eax", "edx");
}


bool Init()
{
    return proc_GetObjectMarkers
        && proc_GetUnitCameraPosition
        && ptr_pObjectPrototypes;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_GetObjectMarkers);
    SENTINEL_DEBUG_VAR("%p", proc_GetUnitCameraPosition);
    SENTINEL_DEBUG_VAR("%p", ptr_pObjectPrototypes);
}

} } // namespace reve::object
