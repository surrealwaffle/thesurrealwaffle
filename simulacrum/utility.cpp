
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "utility.hpp"

#include <algorithm>

#include <sentutil/all.hpp>

namespace simulacrum {

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

bool intersects_coll_pathfinding_spheres(const sentinel::position3d&  segment_begin,
                                         const sentinel::direction3d& segment_direction,
                                         const sentinel::real&        segment_length,
                                         const sentinel::object&      object,
                                         const sentinel::tags::collision_model& coll)
{
    using pathfinding_sphere = sentinel::tags::collision_model::pathfinding_sphere;
    auto test_sphere = [&segment_begin, &segment_direction, &segment_length, &object]
                       (const pathfinding_sphere& sphere) {
        const sentinel::real3d center = sphere.node < 0
            ? object.object.position + sphere.center
            : object.object.node_transforms[sphere.node] * sphere.center;
        return intersects_segment_sphere(segment_begin,
                                         segment_direction,
                                         segment_length,
                                         center,
                                         sphere.radius); // should scale but its ok
    };

    return std::any_of(coll.pathfinding.spheres.begin(),
                       coll.pathfinding.spheres.end(),
                       test_sphere);
}

void visit_map_scenery(const std::function<void(sentinel::object&, sentinel::tags::object&)>& visitor){
    [[maybe_unused]]
    constexpr auto object_type_biped   = 0;
    [[maybe_unused]]
    constexpr auto object_type_vehicle = 1;
    constexpr auto object_type_scenery = 6;

    for (sentinel::object_table_datum& object_dat : sentutil::globals::objects) {
        if (object_dat.type != object_type_scenery)
            continue;

        sentinel::object&       object     = *object_dat.object;
        sentinel::tags::object& definition = *reinterpret_cast<sentinel::tags::object*>(object.object.tag->definition);
        visitor(object, definition);
    }
}

} // namespace simulacrum
