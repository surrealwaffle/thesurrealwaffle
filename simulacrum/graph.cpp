#include "graph.hpp"
#include "utility.hpp"

#include <cmath>

#include <algorithm>
#include <vector>

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/segment.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/range/iterator_range_core.hpp>

#include <sentutil/all.hpp>

namespace {

bool is_surface_navigable(const simulacrum::utility::interface_cbsp_surface& surface);


} // namespace (anonymous)

namespace simulacrum {

boost::geometry::index::rtree<
    std::pair<
        boost::geometry::model::box<sentinel::real3d>,
        collision_hierarchy_entry>,
    boost::geometry::index::rstar<8>>
build_dynamic_collision_hierarchy()
{
    using box_type   = boost::geometry::model::box<sentinel::real3d>;
    using value_type = std::pair<box_type, collision_hierarchy_entry>;
    using policy     = boost::geometry::index::rstar<8>;
    using index_type = boost::geometry::index::rtree<value_type, policy>;

    std::vector<value_type> collidables;

    auto visitor = [&collidables] (sentinel::object&       object,
                                   sentinel::tags::object& definition)
    {
        if (!definition.object.collision_model)
            return;

        collidables.emplace_back(box_type{object.object.bound_center - sentinel::real3d::filled(object.object.bound_radius),
                                          object.object.bound_center + sentinel::real3d::filled(object.object.bound_radius)},
                                 collision_hierarchy_entry{std::ref(object),
                                                           std::ref(definition),
                                                           std::ref(*definition.object.collision_model)});
    };

    visit_map_scenery(visitor);

    return index_type(collidables.cbegin(), collidables.cend());
}

std::size_t navigation_graph_node::hash_value() const
{
    std::size_t seed = 0;
    boost::hash_combine(seed, (int)cbsp_type);
    boost::hash_combine(seed, (int)cbsp_index);
    return seed;
}

navigation_graph::navigation_graph(this_collision_bsp_tag)
    : navigation_graph(utility::interface_cbsp{*sentutil::globals::map_globals->collision_bsp})
{

}

navigation_graph::navigation_graph(const utility::interface_cbsp& cbsp)
    : graph()
    , spatial_index()
{
    using Node = navigation_graph_node;

    std::vector<std::pair<Node, Node>> node_pairs;

    for (const auto& surface : cbsp.surfaces) {
        if (!(is_surface_navigable)(surface))
            continue;

        const Node surface_node {
            surface.centroid(),
            Node::type_surface, surface.index
        };

        for (const auto& edge : surface.edges()) {
            [[maybe_unused]]
            const Node other_surface_node {
                edge.next_surface(surface).centroid(),
                Node::type_surface, edge.next_surface(surface).index
            };

            [[maybe_unused]]
            const Node edge_node {
                edge.centroid(),
                Node::type_edge, edge.index
            };

            node_pairs.push_back({surface_node, edge_node});
            node_pairs.push_back({edge_node, surface_node});
        }
    }

    {   // filter out edges that intersect static scenery
        auto collidables = build_dynamic_collision_hierarchy();
        auto intersects_collidable = [&collidables] (const auto& node_pair) -> bool {
            using box_type   = boost::geometry::model::box<sentinel::real3d>;
            using collidable_value_type = std::pair<box_type, collision_hierarchy_entry>;

            const Node& a = node_pair.first;
            const Node& b = node_pair.second;
            const boost::geometry::model::segment<sentinel::real3d> segment(a.point, b.point);

            auto test_spheres
                = [segment_begin = a.point,
                   segment_direction = normalized(b.point - a.point),
                   segment_length = norm(b.point - a.point)]
                  (const collidable_value_type& indexable) -> bool {
                      const collision_hierarchy_entry& e = indexable.second;
                      const sentinel::object& object = e.object;
                      const sentinel::tags::collision_model& coll = e.collision_model;
                      return intersects_coll_pathfinding_spheres(segment_begin,
                                                                 segment_direction,
                                                                 segment_length,
                                                                 object,
                                                                 coll);
                  };
            auto it = collidables.qbegin(boost::geometry::index::intersects(segment));
            return std::any_of(it, collidables.qend(), test_spheres);
        };

        node_pairs.erase(std::remove_if(node_pairs.begin(), node_pairs.end(), intersects_collidable),
                         node_pairs.end());
    }

    // construct the graph
    graph = graph_type(node_pairs.cbegin(), node_pairs.cend(),
                       [] (const Node& a, const Node& b) -> navigation_graph_edge {
                           const auto delta = b.point - a.point;
                           return {normalized(delta), norm(delta)};
                       });


    { // construct the spatial index
        auto node_to_indexable = [this] (const Node& node) -> spatial_indexable {
            return {{node.point[0], node.point[1], node.point[2]}, graph.find(node)};
        };

        std::vector<spatial_indexable> spatial_indices;
        spatial_indices.reserve(graph.size());
        for (const auto& [node, edge] : graph)
            spatial_indices.push_back(node_to_indexable(node));
        spatial_index = spatial_index_type(spatial_indices.cbegin(),
                                           spatial_indices.cend());
    }
}

std::optional<navigation_graph::iterator>
navigation_graph::nearest_node(const sentinel::real3d& pos) const
{
    spatial_indexable out = {};
    if (spatial_index.query(boost::geometry::index::nearest(pos, 1), &out))
        return out.second;
    return std::nullopt;
}

std::optional<navigation_graph::iterator>
navigation_graph::get_node(const navigation_graph::node_type& node) const
{
    auto it = graph.find(node);
    return it != graph.end() ? std::make_optional(it) : std::nullopt;
}

} // namespace simulacrum

namespace {

bool is_surface_navigable(const simulacrum::utility::interface_cbsp_surface& surface)
{
    // todo: calculate from biped tag
    // todo: ladder flag
    constexpr float pi = 0x1.921FB6p1;
    const auto [normal, d] = surface.plane();
    return dot(normal, {0, 0, 1}) >= std::cos(pi / 4);
}

} // namespace (anonymous)
