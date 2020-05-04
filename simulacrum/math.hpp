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

/** \brief Calculates an aiming vector so that a projectile with a given
 *         \a relative_muzzle_velocity and \a inherited_velocity from the parent
 *         object is in the direction of \a target, from origin.
 */
sentinel::real3d
approximate_compensated_aim(const sentinel::real    relative_muzzle_velocity,
                            const sentinel::real3d& inherited_velocity,
                            const sentinel::real3d& target,
                            std::optional<int> iteration_depth = std::nullopt);

} } // namespace simulacrum::math
