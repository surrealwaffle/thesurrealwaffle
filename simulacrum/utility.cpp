
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "utility.hpp"
#include "math.hpp"

#include <algorithm>

#include <sentutil/all.hpp>

namespace simulacrum {

bool intersects_coll_pathfinding_spheres(const sentinel::position3d&  segment_begin,
                                         const sentinel::direction3d& segment_direction,
                                         const sentinel::real&        segment_length,
                                         const sentinel::object&      object,
                                         const sentinel::tags::collision_model& coll)
{
    using math::intersects_segment_sphere;
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
