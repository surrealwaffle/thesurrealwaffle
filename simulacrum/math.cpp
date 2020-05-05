#include "math.hpp"

#include <cmath>

namespace simulacrum { namespace math {

std::pair<float, float>
get_turn_angles(const OrientationContext& orientation,
                const sentinel::real3d&   target)
{
    const sentinel::matrix2d horizontal_orient {
        orientation.cos_yaw, -orientation.sin_yaw,
        orientation.sin_yaw, orientation.cos_yaw
    };

    const sentinel::matrix2d vertical_orient {
        orientation.cos_pitch, -orientation.sin_pitch,
        orientation.sin_pitch, orientation.cos_pitch
    };

    const auto delta_xy = horizontal_orient * sentinel::real2d{target[0], target[1]};
    const auto delta_z = vertical_orient * sentinel::real2d{norm(delta_xy), target[2]};

    return {std::atan2(delta_xy[1], delta_xy[0]),
            std::atan2(delta_z[1], delta_z[0])};
}

sentinel::real3d
get_initial_projectile_velocity(const sentinel::real    relative_muzzle_velocity,
                                const sentinel::real3d& aiming_direction,
                                const sentinel::real3d& parent_velocity)
{
    return (relative_muzzle_velocity + dot(aiming_direction, parent_velocity)) * aiming_direction;
}

} } // namespace simulacrum::math
