
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <array>
#include <limits>
#include <optional>

#include <sentinel/types.hpp>
#include <sentinel/object.hpp>
#include <sentutil/impl/impl_object.hpp>

namespace sentutil { namespace object {

/** \brief Retrieves information about a marker of an object.
 *
 * \param[in]  object      The identity of the object.
 * \param[in]  marker_name The name of the marker to query.
 *
 * \return A `std::optional` containing marker information on success, or
 *         an empty `std::optional` if the marker information was not retrieved.
 */
std::optional<sentinel::object_marker_result_type>
get_object_marker(const sentinel::identity<sentinel::object_table_datum>& object,
                  const char* marker_name);

template<std::size_t N>
int get_object_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                       const char* marker_name,
                       std::array<sentinel::object_marker_result_type, N>& result);

template<std::size_t N>
int get_object_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                       const char* marker_name,
                       sentinel::object_marker_result_type (&result)[N]);

/** \brief Retrieves a unit's camera position.
 *
 * This function ignores any interpolation on Halo's part, even when called on
 * the local player's unit.
 *
 * \param[in] unit The unit to query.
 *
 * \return The position of the unit's camera, in absolute coordinates.
 */
sentinel::point3d get_unit_camera(const sentinel::identity<sentinel::unit>& unit);

/** \brief Updates the objects in the map by a number of ticks.
 *
 * If \a ticks is less than or equal to zero, this function simply returns.
 *
 * \param[in] ticks The number of ticks to update by.
 */
void update_objects(long ticks);

} } // namespace sentutil::object

// -----------------------------
// Function Template Definitions

namespace sentutil { namespace object {

template<std::size_t N>
int get_object_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                       const char* marker_name,
                       std::array<sentinel::object_marker_result_type, N>& result)
{
    static_assert(N <= std::numeric_limits<sentinel::h_short>::max());
    return impl::get_markers(object,
                             marker_name,
                             result.data(), static_cast<sentinel::h_short>(N));
}

template<std::size_t N>
int get_object_markers(const sentinel::identity<sentinel::object_table_datum>& object,
                       const char* marker_name,
                       sentinel::object_marker_result_type (&result)[N])
{
    static_assert(N <= std::numeric_limits<sentinel::h_short>::max());
    return impl::get_markers(object,
                             marker_name,
                             +result, static_cast<sentinel::h_short>(N));
}

} } // namespace sentutil::object

