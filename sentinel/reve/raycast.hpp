#pragma once

#include "types.hpp"


namespace sentinel {

struct raycast_result_type;

} // namespace sentinel

namespace reve { namespace raycast {

using sentinel::raycast_result_type;

/** \brief Casts a ray from \a pos through a \a delta.
 *
 * \sa proc_CastRay
 *
 * \param[in] mask Determines what the ray may hit. Example values include
 *                 `0xC2AD` when performing vision checks and `0x1000E9` when
 *                 performing projectile collisions.
 * \param[in] pos The position to start the ray from.
 * \param[in] delta The change in position to cast the ray through.
 * \param[in] source_object_id If not `-1`, this object is ignored in raycasts.
 * \param[out] result Stores the output.
 *
 * \return `0` if the ray did not intersect any surface, otherwise non-zero.
 */
using cast_ray_tproc __attribute__((cdecl))
    = bool8(*)(P_IN  mask_long            mask,
               P_IN  real const*          pos,
               P_IN  real const*          delta,
               P_IN  identity_raw         source_object_id,
               P_OUT raycast_result_type* result);

extern cast_ray_tproc proc_CastRay;

bool Init();

void Debug();

} } // namespace reve::raycast
