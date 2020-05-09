#include "math.hpp"

#include <cmath>

#include <algorithm>

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

bool intersects_segment_sphere(const sentinel::position3d&  segment_begin,
                               const sentinel::direction3d& segment_direction,
                               const sentinel::real&        segment_length,
                               const sentinel::position3d&  sphere_center_,
                               const sentinel::real&        sphere_radius)
{
    const auto square = [] (const auto& v) { return v * v; };

    // Translate coordinates so that segment_begin is the origin
    const sentinel::position3d sphere_center = sphere_center_ - segment_begin;
    const auto segment_end = segment_length * segment_direction;
    /* segment_begin = sentinel::real3d::zero; */

    const auto square_sphere_radius = square(sphere_radius);
    const auto center_component_length = dot(segment_direction, sphere_center);
    /*
    const auto center_component = center_component_length * segment_direction;
    const auto center_ortho_component = sphere_center - center_component;
    */

    return (norm2(sphere_center) < square_sphere_radius) ||
           (norm2(segment_end - sphere_center) < square_sphere_radius) ||
           (norm2(sphere_center) - square(center_component_length)) < square_sphere_radius;
}

float
compute_decaying_differential(float x, float dt, float decay_rate, float constant_rate)
{
    const float lbound = std::min(0.0f, x);
    const float ubound = std::max(0.0f, x);

    const float dx = -decay_rate * x * dt - constant_rate * dt;
    return std::clamp(x + dx, lbound, ubound) - x;
}

} } // namespace simulacrum::math
