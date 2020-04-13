#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

#include <sentinel/fwd/table_fwd.hpp>
#include <sentinel/structures/raycast.hpp>

extern "C" {

/** Casts a ray from \a source to an offset from \a source by \a delta.
 *
 * I do not yet know the exact flags of \a mask, however these values are known:
 *  * `0xC2AD` is used when performing vision checks (as in flag visibility, etc)
 *  * `0x1000E9` is used when performing projectile point-collision checks
 *
 * \param[in] mask Affects what is included in the ray test.
 * \param[in] source Starting point of the ray.
 * \param[in] delta Determines the endpoint of the ray by `*source + *delta`.
 * \param[in] source_object Identity of the object to ignore in the ray test or the invalid identity.
 * \param[out] result The result of the ray test.
 * \return `true` if the ray intersected a surface, otherwise `false`.
 */
SENTINEL_API
bool
sentinel_Raycast_Test(sentinel::mask_long                  mask,
                      sentinel::position3d const*          source,
                      sentinel::position3d const*          delta,
                      sentinel::identity<sentinel::object_table_datum> object_id,
                      sentinel::raycast_result_type* result);

} // extern "C"
