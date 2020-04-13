#ifndef SIMULA__NAV_HPP
#define SIMULA__NAV_HPP

#include <vector>
#include <unordered_map>

#include <sentinel/numeric_types.hpp>

#include "range.hpp"

namespace simula::nav {

    using node_index = std::size_t;

    /** \brief Represents a navigation point in the graph. */
    struct node;

    /** \brief The collection of navigational nodes and links between nodes, indicating that traversal is possible. */
    struct graph;

    struct alignas(32) node {

        struct {
            sentinel::uint32 index   : 31; ///< The element index in the collision BSP.
            sentinel::uint32 is_edge :  1; ///< If set, indicates #index refers to an edge, otherwise #index refers to a surface.
            sentinel::real3  point;        ///< The node position in the map.
        } world;

    };

    struct graph {
        struct alignas(8) adjacency_information {
            node_index  start; ///< The index in #neighbors from which to begin reading neighbors.
            std::size_t count; ///< The number of neighbors to read from #start.
        };

        using nodes_container     = std::vector<node>;
        using index_container     = std::vector<node_index>;
        using adjacency_container = std::vector<adjacency_information>;
        using distance_container  = std::vector<sentinel::real>;
        using surface_mapping     = std::unordered_map<sentinel::int32, node_index>;

        nodes_container     nodes;
        index_container     neighbors;
        adjacency_container adjacency;
        distance_container  distance_matrix;
        surface_mapping     surface_map;

        range<node_index const> neighbors_of(node_index const index) const {
            adjacency_information const& ainfo = adjacency[index];
            auto* const start = neighbors.data() + ainfo.start;
            return range(start, start + ainfo.count);
        }

        void shrink_to_fit() {
            nodes.shrink_to_fit();
            neighbors.shrink_to_fit();
            adjacency.shrink_to_fit();
            distance_matrix.shrink_to_fit();
        }

        void clear() noexcept {
            surface_map.clear();
            distance_matrix.clear();
            adjacency.clear();
            neighbors.clear();
            nodes.clear();
        }

        sentinel::real&       distance(node_index const from, node_index const to)       { return distance_matrix[from * nodes.size() + to]; }
        sentinel::real const& distance(node_index const from, node_index const to) const { return distance_matrix[from * nodes.size() + to]; }

        void calculate_distances();

        bool build();
    };
}

#endif // SIMULA__NAV_HPP
