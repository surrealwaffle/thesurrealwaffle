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
approximate_compensated_aim(const sentinel::real    relative_muzzle_velocity,
                            const sentinel::real3d& inherited_velocity,
                            const sentinel::real3d& target,
                            std::optional<int> iteration_depth_)
{
    //constexpr float muzzle_frac = 0.75f;
    const float distance = norm(target);
    //const float reciprocal_distance = 1 / distance;
    const sentinel::real3d target_direction = normalized(target);

    sentinel::real3d guess = target_direction;

    for (int i = iteration_depth_.value_or(8); i; --i) {
        const sentinel::real3d velocity = relative_muzzle_velocity * guess + inherited_velocity;
        const sentinel::real3d projectile_direction = normalized(velocity);
        guess = normalized(distance * guess + (target - distance * projectile_direction));
    }

    return guess;
}

} } // namespace simulacrum::math
