
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

#include <sentinel/fwd/table_fwd.hpp>
#include <sentinel/structures/table.hpp>
#include <sentinel/structures/object.hpp>
#include <sentinel/structures/item.hpp>
#include <sentinel/structures/weapon.hpp>
#include <sentinel/structures/unit.hpp>
#include <sentinel/structures/biped.hpp>

extern "C" {

/** \brief Retrieves the markers of an object.
 *
 * As marker names are not unique, this function is able to output more than one
 * result to \a out, up to the limit allowed by \a out_count.
 *
 * \param[in] object The identifier of the object.
 * \param[in] marker_name The name of the marker.
 * \param[out] out An array of structures to receive the marker information.
 * \param[out] out_count The maximum number of results to output to \a out.
 *
 * \return The number of results output to \a out.
 */
SENTINEL_API
int
sentinel_Object_GetMarkers(sentinel::identity<sentinel::object_table_datum> object_id,
                           sentinel::h_ccstr                    marker_name,
                           sentinel::object_marker_result_type* out,
                           sentinel::h_short                    out_count);

/** \brief Retrieves the camera position of a unit.
 *
 * Calls on the same function that retrieves the camera position when using
 * `(camera_set_first_person <unit>)`.
 *
 * Alternatively, it can be used as a safe way of retrieving the non-interpolated
 * camera position of the local player.
 *
 * \param[in] unit_id The identifier of the unit.
 * \param[out] position Receives the camera position.
 */
SENTINEL_API
void
sentinel_Unit_GetCameraPosition(sentinel::identity<sentinel::unit> unit_id,
                                sentinel::real3d*                  position);

/** \brief Returns in the structures supplied positioning data of a local unit
 *         extrapolated out by \a seconds.
 *
 * \return `true` if positioning data was written, otherwise `false`.
 */
SENTINEL_API
bool
sentinel_Engine_ExtrapolateLocalUnitDelta(sentinel::real3d* delta,
                                          sentinel::real3d* unknown0,
                                          sentinel::real3d* unknown1,
                                          sentinel::real    seconds);

/** \brief Updates all objects (and netgame flags, for teleporters) by \a ticks.
 *
 * This is not a true update, as it passes on many other update routines
 *
 * \param[in] ticks The number of ticks to advance by. May be non-positive,
 *                  in which case this call simply returns.
 */
SENTINEL_API
void
sentinel_Object_UpdateObjects(long ticks);

/** \brief Updates a biped's position.
 *
 * \param[in] biped_id The biped to update.
 * \param[in] ticks The number of ticks to update through.
 */
SENTINEL_API
void
sentinel_Biped_UpdateBipedPosition(sentinel::identity<sentinel::biped> biped_id);

} // extern "C""
