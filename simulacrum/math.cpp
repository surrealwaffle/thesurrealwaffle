#include "math.hpp"
#include "game_context.hpp"

#include <cmath>

#include <algorithm>
#include <sentutil/console.hpp>

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
initial_projectile_velocity(const sentinel::real    muzzle_speed,
                            const sentinel::real3d& aiming_direction,
                            const sentinel::real3d& parent_velocity)
{
    // This is an approximation:
    // Initial projectile direction is determined by the following expression:
    //    normalized(parent_velocity + muzzle_speed * aiming_direction).
    // Then the initial projectile velocity is determined by:
    //    (muzzle_speed + dot(aiming_direction, parent_velocity)) * initial_direction
    // However, parent velocity is generally small enough laterally that we can approximate
    // the initial projectile direction with the aiming direction.
    // Ideally, an 'accurate model' option should later be provided for simulacrum that accounts for this.
    // This model would need to modulate the aiming direction so that the initial
    // projectile direction is sufficiently correct and on target.
    return (muzzle_speed + dot(aiming_direction, parent_velocity)) * aiming_direction;
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
decaying_differential(const float x,
                      const float dt,
                      const float decay_rate,
                      const float constant_rate)
{
    const float lbound = std::min(0.0f, x);
    const float ubound = std::max(0.0f, x);

    const float dx = -decay_rate * x * dt + (std::signbit(x) ? constant_rate : -constant_rate) * dt;
    return std::clamp(x + dx, lbound, ubound) - x;
}

std::pair<float, float>
projectile_interpolation_time(const ProjectileContext& projectile,
                              const float initial_speed)
{
    // Per tick, a projectile's speed is reduced by a constant amount if its speed
    // is greater than the projectile's final velocity value in the tag.
    // This value, known as the lerp constant, is calculated by:
    //  `(v_f + v_i)/2 * (v_f - v_i)/(r_f - r_i)`,
    // where v_f, v_i describe the final and initial projectile speed and r_f, r_i
    // describe the (air/water) damage range values from the tag.
    // It is therefore possible and likely that a projectile will fall below its
    // final velocity, as the projectile speed is not bounded below by v_f.
    // If `v_f == v_i` or `r_f == r_i` then the projectile does not lerp.

    if (!projectile.does_lerp || initial_speed <= projectile.speed_final)
        return {0L, initial_speed};

    const float ticks = std::ceil((projectile.speed_final - initial_speed) * projectile.reciprocal_lerp_constant);
    return {ticks, initial_speed + ticks * projectile.lerp_constant};
}

float
projectile_interpolation_distance(const ProjectileContext& projectile,
                                  const float initial_speed)
{
    const auto [ticks, final_speed] = projectile_interpolation_time(projectile, initial_speed);
    return ticks * initial_speed + (ticks * ticks) * projectile.half_lerp_constant;
}

std::pair<bool, float>
projectile_travel_time(const ProjectileContext& projectile,
                       const float initial_speed,
                       const float distance)
{
    if (distance >= projectile.max_range)
        return {false, 0.0f};

    const auto squared = [] (const auto& x) { return x * x; };
    const auto [interpolation_time, final_speed] = projectile_interpolation_time(projectile, initial_speed);
    const float interpolation_distance = projectile_interpolation_distance(projectile, initial_speed);

    const float travel_time = distance < interpolation_distance
        ? (std::sqrt(squared(initial_speed) + 2 * projectile.lerp_constant * distance) - initial_speed) * projectile.reciprocal_lerp_constant
        : interpolation_time + (distance - interpolation_distance) / final_speed;

    return {true, std::ceil(travel_time)};
}

} } // namespace simulacrum::math
