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

/** \brief Computes `x(t + dt) - x(t)` where `x` is determined by the differential
 *         equation `dx = -r * x * dt - k * dt` with the constraint that `x(t + dt)`
 *         is bounded by `0` and `x(t)`, for a differential \a dt.
 *
 * \param[in] x          The value of `x(t)`.
 * \param[in] dt         The time differential.
 * \param[in] decay_rate The constant `r` in the above differential equation.
 * \param[in] time_rate  The constant `k` in the above differential equation.
 * \return The value of `x(t + dt)` according to the constraints.
 */
float
compute_decaying_differential(float x, float dt, float decay_rate, float constant_rate);

} } // namespace simulacrum::math
