#pragma once

#include <optional>

#include <sentinel/types.hpp>
#include <sentinel/raycast.hpp>

namespace sentutil { namespace raycast {

using sentinel::identity;
using sentinel::invalid_identity;
using sentinel::object_table_datum;

/** \brief Casts a ray from a position across a delta.
 *
 * \param[in] mask    Determines what surface and objects to test.
 * \param[in] source  The starting position of the ray.
 * \param[in] delta   The translation to move the ray through.
 * \param[in] exclude If valid, the raycast ignores this object.
 *
 * \return A `std::optional` containing ray intersection information if the
 *         ray intersected a surface as determined by \a mask, otherwise
 *         an empty `std::optional`.
 */
std::optional<sentinel::raycast_result_type>
cast_ray(sentinel::mask_long      mask,
         const sentinel::point3d& source,
         const sentinel::point3d& delta,
         const identity<object_table_datum>& exclude = invalid_identity);

/** \brief Casts a ray from a position across a delta, testing surfaces and objects
 *         as if the ray was a projectile.
 *
 * This is the same call used to get intersection information for projectiles.
 *
 * \param[in] source  The starting position of the ray.
 * \param[in] delta   The translation to move the ray through.
 * \param[in] exclude If valid, the raycast ignores this object.
 *
 * \return A `std::optional` containing ray intersection information if the
 *         ray intersected a tested surface, otherwise an empty `std::optional`.
 */
inline
std::optional<sentinel::raycast_result_type>
cast_projectile_ray(const sentinel::point3d& source,
                    const sentinel::point3d& delta,
                    const identity<object_table_datum>& exclude = invalid_identity)
{ return cast_ray(0x1000E9, source, delta, exclude); }

/** \brief Casts a ray from a position across a delta, testing surfaces and objects
 *         as if the ray was testing for visibility.
 *
 * This is the same call used to determine if a flag or objective is visible.
 *
 * \param[in] source  The starting position of the ray.
 * \param[in] delta   The translation to move the ray through.
 * \param[in] exclude If valid, the raycast ignores this object.
 *
 * \return A `std::optional` containing ray intersection information if the
 *         ray intersected a tested surface, otherwise an empty `std::optional`.
 */
inline
std::optional<sentinel::raycast_result_type>
cast_vision_ray(const sentinel::point3d& source,
                const sentinel::point3d& delta,
                const identity<object_table_datum>& exclude = invalid_identity)
{ return cast_ray(0xC2AD, source, delta, exclude); }

} } // namespace sentutil::raycast
