#include "nav.hpp"

#include <cmath>

#include <unordered_map>
#include <queue>
#include <limits>

#include <sentinel/globals.hpp>
#include <sentinel/tags/scenario.hpp>
#include <sentinel/tags/collision_bsp.hpp>

using namespace sentinel;

namespace {

    real constexpr BIG = std::numeric_limits<real>::max();

}

namespace simula::nav {

bool graph::build() {
    clear();
    using surface_t = tags::collision_bsp::surface;
    using edge_t = tags::collision_bsp::edge;

    globals::map_globals const* map_globals = sentinel__Globals_GetMapGlobals();
    if (!map_globals)
        return false;

    tags::scenario const *scenario  = map_globals->scenario;
    if (!scenario)
        return false;

    if (auto* sbsp_index = sentinel__Globals_GetBSPIndex(); !sbsp_index || *sbsp_index < 0)
        return false;

    tags::structure_bsp const *sbsp = map_globals->structure_bsp;
    if (!sbsp || sbsp->collision_bsps.count == 0)
        return false;

    tags::collision_bsp const& cbsp = sbsp->collision_bsps[0];

    auto is_navable = [&cbsp] (surface_t const& surface) {
        constexpr real sqrt2_2 = std::sqrt(2.0) / 2.0;

        real3 const& plane_normal = cbsp.planes[surface.plane_index()].normal;
        bool const is_reversed = surface.is_reversed();
        bool const is_walkable = (is_reversed ? -plane_normal[2] : plane_normal[2]) >= sqrt2_2;

        return is_walkable || surface.test_flags(surface.FLAGS::CLIMBABLE);
    };

    auto foreach_edge = [&cbsp] (int32 const surface_index, auto&& op) {
        surface_t const& surface = cbsp.surfaces[surface_index];
        int32 const first_edge = surface.first_edge;
        int32 edge_index = first_edge;

        do {
            edge_t const& edge = cbsp.edges[edge_index];
            op(edge_index, edge);
            edge_index = edge.next_edge(surface_index);
        } while(edge_index != first_edge);
    };

    auto foreach_adjacent_navable_surface = [&cbsp, &foreach_edge, &is_navable] (int32 const surface_index, auto&& op) {
        auto new_op = [&cbsp, &op, &is_navable, surface_index] (int32 const, edge_t const& edge) {
            int32 const adjacent_surface_index = edge.adjacent_surface(surface_index);
            surface_t const& adjacent_surface = cbsp.surfaces[adjacent_surface_index];
            if (is_navable(adjacent_surface)) {
                op(adjacent_surface_index);
            }
        };
        foreach_edge(surface_index, new_op);
    };

    auto get_centroid = [&cbsp, &foreach_edge] (int32 const surface_index) {
        real3 centroid = {0, 0, 0};
        int vertices = 0;
        auto add_start_vertex = [&cbsp, surface_index, &centroid, &vertices] (int32, edge_t const& edge) {
            real3 const& vertex_point = cbsp.vertices[edge.start_vertex(surface_index)].point;
            for (int i = 0; i < 3; ++i)
                centroid[i] += vertex_point[i];
            ++vertices;
        };
        foreach_edge(surface_index, add_start_vertex);
        for (auto&& f : centroid) f /= vertices;
        return centroid;
    };

    struct adjacency_build_info {
        std::size_t        node;
        std::vector<int32> neighbor_surfaces;
    };

    std::unordered_map<int32, adjacency_build_info> adjacency_pregraph;
    for (int32 surface_index = 0; surface_index < cbsp.surfaces.count; ++surface_index) {
        surface_t const& surface = cbsp.surfaces[surface_index];
        if (!is_navable(surface))
            continue;

        // iterate over surface edges
        //   1. get adjacent navable surfaces
        //   2. get centroid
        real3 const centroid = get_centroid(surface_index);
        std::vector<int32> neighbor_surfaces;

        foreach_adjacent_navable_surface(surface_index, [&neighbor_surfaces] (int32 neighbor_index) { neighbor_surfaces.push_back(neighbor_index); });

        std::size_t const node_index = nodes.size();
        nodes.push_back(node{static_cast<uint32>(surface_index), 0, centroid});
        adjacency_pregraph[surface_index] = adjacency_build_info{node_index, std::move(neighbor_surfaces)};
    }

    adjacency.resize(adjacency_pregraph.size(), {0, 0});
    for (auto const& e : adjacency_pregraph) {
        int32 const                 surface_index = e.first;
        adjacency_build_info const& build_info    = e.second;

        surface_map[surface_index] = build_info.node;

        std::size_t start = neighbors.size();
        std::size_t count = 0u;
        for (int32 const neighbor_surface : build_info.neighbor_surfaces) {
            if (!adjacency_pregraph.count(neighbor_surface)) {
                printf("unprocessed neighbor surface: %d\n", neighbor_surface);
                continue;
            }
            adjacency_build_info const& build_info = adjacency_pregraph[neighbor_surface];
            neighbors.push_back(build_info.node);
            ++count;
        }
        adjacency[build_info.node] = {start, count};
    }

    shrink_to_fit();
    calculate_distances();

    return true;
}

void graph::calculate_distances() {
    auto linear_distance = [] (real3 const& a, real3 const& b) {
        real3 const diff {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
        return std::sqrt(diff[0] * diff[0] + diff[1] * diff[1] + diff[2] * diff[2]); // don't need sqrt, but it can be afforded
    };

    auto linear_distance_indexed = [this, &linear_distance]  (node_index const a, node_index const b) {
        return linear_distance(nodes[a].world.point, nodes[b].world.point);
    };

    // floyd-warshall is too slow
    // just use dijkstra's
    std::size_t const NODES = nodes.size();

    printf("\tbuilding distance matrix\n");
    struct alignas(8) dijkstra_node {
        real        dist; ///< The distance from the source_node to #node.
        std::size_t node;

        constexpr bool operator<(dijkstra_node const& other) const noexcept { return dist > other.dist; }
    };

    distance_matrix.clear();
    distance_matrix.resize(nodes.size() * nodes.size(), BIG);

    std::priority_queue<dijkstra_node> queue;
    std::vector<bool>                  visited(NODES, false);
    std::vector<real>                  distances(NODES);

    for (std::size_t source_node = 0; source_node < NODES; ++source_node) {
        //printf("\tsource:%u/%u\n", source_node, NODES - 1u);

        for (auto&& b : visited)
            b = false;

        queue.push(dijkstra_node{0, source_node});
        while (!queue.empty()) {
            auto const& n = queue.top();

            if (!visited[n.node]) {
                //printf("\t\tvisiting %u (%u): %f\n", n.node, nodes[n.node].world.index, n.dist);
                visited[n.node] = true;
                distance(source_node, n.node) = n.dist;
                for (auto const neighbor_node : neighbors_of(n.node)) {
                    //printf("\t\t\tpotential neighbor: %u (%u)\n", neighbor_node, nodes[neighbor_node].world.index);
                    if (!visited[neighbor_node]) {
                        //printf("\t\t\t\tpushed\n");
                        auto const distance_to_neighbor = linear_distance_indexed(n.node, neighbor_node);
                        queue.push(dijkstra_node{n.dist + distance_to_neighbor, neighbor_node});
                    }
                }
            } else if (n.dist < distance(source_node, n.node)) {
                distance(source_node, n.node) = n.dist;
            }

            queue.pop();
        }
    }

    printf("\t--finished distance matrix\n");
}

}
