#pragma once

#include <utility> // std::pair

#include <sentinel/types.hpp>
#include <sentinel/structures/object.hpp>

#include "game_context.hpp"

namespace simulacrum { namespace math {

/** \brief Computes the angles to turn through in order for the forward direction from
 *         \a orientation to be reoriented to point at \a target from origin.
 *
 * \return The yaw-pitch angle pair to turn through.
 */
std::pair<float, float>
get_turn_angles(const OrientationContext& orientation,
                const sentinel::real3d&   target);

/** \brief Computes the angles to turn through in order for the forward direction from
 *         \a orientation to be reoriented to point at \a target from \a source.
 *
 * \return The yaw-pitch angle pair to turn through.
 */
inline
std::pair<float, float>
get_turn_angles(const sentinel::real3d&   source,
                const OrientationContext& orientation,
                const sentinel::real3d&   target)
                { return get_turn_angles(orientation, target - source); }

/** \brief Calculates the initial velocity of a projectile fired towards
 *         \a aiming_direction with \a relative_muzzle_velocity from an object
 *         with velocity \a parent_velocity.
 */
sentinel::real3d
get_initial_projectile_velocity(const sentinel::real    relative_muzzle_velocity,
                                const sentinel::real3d& aiming_direction,
                                const sentinel::real3d& parent_velocity);
/** \brief Tests if a line segment intersects the sphere.
 *
 * \return `true` if the line segment intersects the sphere, otherwise `false`.
 */
bool
intersects_segment_sphere(const sentinel::position3d&  segment_begin,
                          const sentinel::direction3d& segment_direction,
                          const sentinel::real&        segment_length,
                          const sentinel::position3d&  sphere_center,
                          const sentinel::real&        sphere_radius);

} } // namespace simulacrum::math
