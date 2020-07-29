#pragma once

#include <utility> // std::pair

#include <sentinel/types.hpp>

namespace simulacrum {
    struct OrientationContext;
    struct ProjectileContext;
}

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

/** \brief Calculates the initial velocity of a projectile.
 *
 * \param[in] muzzle_speed     The initial speed of the projectile as fired from rest.
 * \param[in] aiming_direction The direction the weapon is aiming towards.
 * \param[in] parent_velocity  The velocity of the firing weapon.
 * \return The initial velocity of a projectile of the given parameters.
 */
sentinel::real3d
initial_projectile_velocity(const sentinel::real    muzzle_speed,
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
 * \return An approximation for `dx` according to the constraints.
 */
float
decaying_differential(const float x,
                      const float dt,
                      const float decay_rate,
                      const float constant_rate);

/** \brief Calculates the number of ticks that a projectile's velocity lerps through.
 *
 * This calculation ignores gravity.
 *
 * \param[in] projectile    The context for the projectile definition.
 * \param[in] initial_speed The initial speed of the projectile.
 * \return A pair containing the number of ticks to interpolate through (first)
 *         and the projectile's speed after interpolation (second).
 */
std::pair<float, float>
projectile_interpolation_time(const ProjectileContext& projectile,
                              const float initial_speed);

/** \brief Calculates the distance that a projectile's speed interpolates through
 *         from \a initial_speed.
 *
 * \param[in] projectile    The context for the projectile definition.
 * \param[in] initial_speed The initial speed of the projectile.
 * \return The number of Halo units \a projectile interpolates through.
 */
float
projectile_interpolation_distance(const ProjectileContext& projectile,
                                  const float initial_speed);

/** \brief Calculates the time needed for a projectile to travel a certain distance.
 *
 * \param[in] projectile    The context for the projectile definition.
 * \param[in] initial_speed The initial speed of the projectile.
 * \param[in] distance      The goal distance for the projectile to  travel.
 * \return A pair indicating the projectile travels \a distance before detonating (first)
 *         and the number of partial ticks to travel \a distance (second).
 */
std::pair<bool, float>
projectile_travel_time(const ProjectileContext& projectile,
                       const float initial_speed,
                       const float distance);

} } // namespace simulacrum::math
