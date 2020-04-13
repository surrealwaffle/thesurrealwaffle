#pragma once

#include <cstddef>

#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <sentinel/structures/object.hpp>
#include <sentinel/tags/object.hpp>
#include <sentinel/tags/collision_model.hpp>

#include "utility.hpp"
#include "bsp_interface.hpp"

namespace simulacrum {

struct this_collision_bsp_tag { };
inline constexpr this_collision_bsp_tag this_collision_bsp = {};

struct collision_hierarchy_entry {
    std::reference_wrapper<sentinel::object> object;
    std::reference_wrapper<sentinel::tags::object> object_definition;
    std::reference_wrapper<sentinel::tags::collision_model> collision_model;
};

boost::geometry::index::rtree<
    std::pair<
        boost::geometry::model::box<sentinel::real3d>,
        collision_hierarchy_entry>,
    boost::geometry::index::rstar<8>>
build_dynamic_collision_hierarchy();

/** \brief Implements an adjacency list that cannot be modified once constructed.
 *
 * The user is required to implement the equality comparison operator for \a Node and
 * the specialization `std::hash<Node>`.
 */
template<
    class Node,
    class Edge = empty_struct
> class compiled_adjacency_list {
public:
    struct edge_type;
    using edge_iterator  = const edge_type*;
    using edge_list_type = rough_span<edge_iterator>;
    using nodes_container_type = std::unordered_map<Node, edge_list_type>;
    using edge_container_type  = std::vector<edge_type>;

    using hasher     = typename nodes_container_type::hasher;
    using size_type  = typename nodes_container_type::size_type;
    using value_type = typename nodes_container_type::value_type;

    /** \brief The iterator type for the elements of the adjacency structure.
     *         Elements referred to by such iterators are constant pairings of
     *         `(Node, edge_list_type)`.
     */
    using iterator  = typename nodes_container_type::const_iterator;


    /** \brief Represents a directed edge in the graph.
     */
    struct edge_type {
        iterator source; ///< The node the edge starts from.
        iterator target; ///< The node the edge goes to.
        Edge     user;   ///< User data for the edge.

        /** \brief Conversion to user edge data.
         */
        operator Edge&() noexcept { return user; }

        /** \brief Conversion to user edge data.
         */
        operator const Edge&() const noexcept { return user; }

        /** \brief Member access for user edge data.
         */
        Edge* operator->() noexcept { return std::addressof(user); }

        /** \brief Member access for user edge data.
         */
        const Edge* operator->() const noexcept { return std::addressof(user); }
    };

    /** \brief Constructs to an empty graph.
     */
    compiled_adjacency_list() = default;

    /** \brief Constructs a graph where the structure is taken in as node pairings.
     *
     * The edges are described as \a Node pairings `(x,y)`, with the range of pairings
     * supplied through `[neighbors_begin, neighbors_end)`.
     * Every node `x` and `y` is interned and the edges are formed by the expression
     * `form_edge(u, v)` where `u`, `v` and the interned counterparts of `x`, `y`.
     *
     * From the time the edges are formed, it is guaranteed that references to the
     * interned nodes will not be invalidated, until the destructor is called.
     */
    template<class ForwardIt, class FormEdge>
    compiled_adjacency_list(ForwardIt neighbors_begin, ForwardIt neighbors_end,
                            const FormEdge& form_edge);

    /** \brief Accesses the internal node counterpart for \a node.
     *
     * Has constant (on average) time-complexity.
     *
     * \return An optional containing a constant reference to the interned node, or
     *         `std::nullopt` if \a node is not interned.
     */
    std::optional<std::reference_wrapper<const Node>>
    intern(const Node& node) const;

    /** \brief Queries the graph to determine if there is a directed edge from
     *         one node to another.
     *
     * Has logarithmic time-complexity.
     * For consistency with #intern, this returns references to the user-domain
     * \a Edge, not the implementation #edge_type.
     *
     * \return An optional containing a constant reference to the edge, or
     *         `std::nullopt` if there is no directed edge between \a from and \a to.
     */
    std::optional<std::reference_wrapper<const Edge>>
    adjacent(const Node& from, const Node& to) const;

    /** \brief Accesses the outgoing edges of a node.
     *
     * Has constant (on average) time-complexity.
     * Unlike #adjacent, the
     *
     * \return An iterable span of the outgoing edges of \a node, or
     *         an empty span if \a node is not in the graph.
     */
    edge_list_type
    egress_edges(const Node& node) const;

    /** \brief Returns an iterator to the first element of the adjacency structure.
     */
    iterator begin() const noexcept { return nodes.begin(); }

    /** \brief Returns an iterator to the element following the last element of the
     *         adjacency structure.
     */
    iterator end() const noexcept { return nodes.end(); }

    /** \brief Gets an iterator to a given node.
     *
     * \return An iterator to the graph vertex of \a node if it is in the graph, or
     *         `end()` if \a node is not in the graph.
     */
    iterator find(const Node& node) const noexcept { return nodes.find(node); }

    /** \brief Returns the number of nodes in the graph.
     */
    size_type size() const noexcept { return nodes.size(); }

    /** \brief Returns the function that hashes the nodes.
     */
    hasher hash_function() const { return nodes.hash_function(); }

private:
    nodes_container_type nodes; ///< A mapping between nodes and sorted, contiguous
                                ///< regions in #edges consisting of node out-edges.
    edge_container_type  edges; ///< A collection of edges between the graph nodes.
                                ///< Internally sorted by
};

struct navigation_graph_node {
    sentinel::real3d point; ///< The node position.

    enum {
        type_surface, ///< Indicates #bsp_index refers to a surface in the CBSP.
        type_edge     ///< Indicates #bsp_index refers to an edge in the CBSP.
    } cbsp_type;     ///< Determines the type of element #cbsp_index refers to.
    long cbsp_index; ///< The index of the CBSP element this node is based on.

    bool operator==(const navigation_graph_node& other) const noexcept {
        return cbsp_type == other.cbsp_type && cbsp_index == other.cbsp_index;
    }

    bool operator!=(const navigation_graph_node& other) const noexcept {
        return !(*this == other);
    }

    std::size_t hash_value() const;
};

} // namespace simulacrum

// --------------------------------------------------
// std::hash specialization for navigation_graph_node

namespace std {

template<>
struct hash<::simulacrum::navigation_graph_node> {
    std::size_t
    operator()(const ::simulacrum::navigation_graph_node& node) const noexcept
    {
        return node.hash_value();
    }
};

} // namespace std

namespace simulacrum {

struct navigation_graph_edge {
    sentinel::direction3d direction;
    float                 distance;
};

class navigation_graph {
public:
    using node_type = navigation_graph_node;
    using edge_type = navigation_graph_edge;
    using graph_type = compiled_adjacency_list<node_type, edge_type>;
    using iterator = graph_type::iterator;

    navigation_graph() = default;

    navigation_graph(const utility::interface_cbsp& cbsp);

    navigation_graph(this_collision_bsp_tag);

    iterator begin() const noexcept { return graph.begin(); }
    iterator end()   const noexcept { return graph.end();   }

    std::optional<iterator>
    nearest_node(const sentinel::real3d& pos) const;

    std::optional<iterator>
    get_node(const node_type& node) const;

    const graph_type&
    get_graph() const { return graph; }

private:
    using spatial_point_type
        = sentinel::real3d;
    using spatial_indexable
        = std::pair<spatial_point_type, iterator>;
    using spatial_index_type
        = boost::geometry::index::rtree<spatial_indexable,
                                        boost::geometry::index::rstar<8>>;

    graph_type         graph;
    spatial_index_type spatial_index;
};

template<class Node, class Edge>
template<class ForwardIt, class FormEdge>
compiled_adjacency_list<Node, Edge>::compiled_adjacency_list(
        ForwardIt neighbors_begin, ForwardIt neighbors_end,
        const FormEdge& form_edge)
    : nodes()
    , edges()
{
    // insert nodes with empty spans, to allocate
    for (auto it = neighbors_begin; it != neighbors_end; ++it) {
        const auto& [node, neighbor] = *it;

        nodes.insert({node,     edge_list_type{nullptr, nullptr}});
        nodes.insert({neighbor, edge_list_type{nullptr, nullptr}});
    }

    // form edges
    edges.reserve(std::distance(neighbors_begin, neighbors_end));
    for (auto it = neighbors_begin; it != neighbors_end; ++it) {
        const auto& [node, neighbor] = *it;
        const auto interned_node_it = nodes.find(node);
        const auto interned_neighbor_it = nodes.find(neighbor);
        edges.push_back({interned_node_it,
                         interned_neighbor_it,
                         form_edge(interned_node_it->first,
                                   interned_neighbor_it->first)});
    }

    auto edge_source = [] (const edge_type& edge) -> const Node& { return edge.source->first; };
    auto edge_target = [] (const edge_type& edge) -> const Node& { return edge.target->first; };
    auto interned_cmp = [] (const Node& a, const Node& b) -> bool { return std::addressof(a) < std::addressof(b); };

    auto cmp_edge_sources = [&edge_source, &interned_cmp] (const edge_type& a, const edge_type& b) {
        return interned_cmp(edge_source(a), edge_source(b));
    };
    auto cmp_edge_targets = [&edge_target, &interned_cmp] (const edge_type& a, const edge_type& b) {
        return interned_cmp(edge_target(a), edge_target(b));
    };

    // sort edges into spans sorted on the source node
    std::sort(edges.begin(), edges.end(), cmp_edge_sources);

    // properly set the edge list spans in the map
    for (auto& [node, span] : nodes) {
        auto [begin, end] = std::equal_range(boost::transform_iterator(edges.begin(), edge_source),
                                             boost::transform_iterator(edges.end(),   edge_source),
                                             node, interned_cmp);
        span = edge_list_type {
            std::addressof(*(begin.base())),
            std::addressof(*(end.base()))
        };

        // put the edge list in sorted order, for lookup later
        std::sort(begin.base(), end.base(), cmp_edge_targets);
    }
}

template<class Node, class Edge>
std::optional<std::reference_wrapper<const Node>>
compiled_adjacency_list<Node, Edge>::intern(const Node& node) const
{
    if (auto it = nodes.find(node); it != nodes.end())
        return std::ref(it->first);
    return std::nullopt;
}

template<class Node, class Edge>
std::optional<std::reference_wrapper<const Edge>>
compiled_adjacency_list<Node, Edge>::adjacent(const Node& from, const Node& to_) const
{
    auto to = intern(to_);
    auto source_it = nodes.find(from);

    if (source_it == nodes.end() || !to)
        return std::nullopt;

    const auto& [node, span] = *source_it;
    auto cmp = [] (const edge_type& edge, const Node& node) {
        return std::addressof(edge.target->first) < std::addressof(node);
    };
    auto edge_it = std::lower_bound(span.begin(), span.end(), to.value(), cmp);
    return edge_it != span.end() ? std::ref(edge_it->user) : std::nullopt;
}

template<class Node, class Edge>
typename compiled_adjacency_list<Node, Edge>::edge_list_type
compiled_adjacency_list<Node, Edge>::egress_edges(const Node& node) const
{
    auto it = nodes.find(node);
    return it != nodes.end() ? it->second
                             : edge_list_type{edges.end(), edges.end()};
}

template<
    class MapIterator,
    class ValueKeyHasher = void
> struct map_iterator_hasher {
    using key_reference  = decltype(std::declval<MapIterator>()->first);
    using key_value_type = std::remove_cv_t<std::remove_reference_t<key_reference>>;
    using hasher_type = std::conditional_t<
                            std::is_void_v<ValueKeyHasher>,
                            std::hash<key_value_type>,
                            ValueKeyHasher>;

    hasher_type hasher = hasher_type();

    std::size_t operator()(const MapIterator& it) const {
        return hasher(it->first);
    }
};

template<class Vertex, class Distance>
struct astar_search_entry {
    Vertex   predecessor;
    Distance distance; // from start
    bool open;
};

template<
    class Node, class Edge,
    class Heuristic,
    class Visitor,
    class EdgePredicate,
    class Distance = decltype(std::declval<const Heuristic&>()(std::declval<const Node&>(), std::declval<const Node&>())),
    class Vertex   = typename compiled_adjacency_list<Node, Edge>::iterator
>
std::unordered_map<
    Vertex,
    astar_search_entry<Vertex, Distance>,
    map_iterator_hasher<Vertex>
>
astar_search(const compiled_adjacency_list<Node, Edge>& graph,
             const Vertex& start,
             const Vertex& goal,
             const Heuristic& heuristic, // invoked as heuristic(node, goal_node)
             Visitor visitor,  // invoked as visitor(predecessor_node, node), returns false to halt evaluation
             const EdgePredicate& edge_predicate) // invoked as edge_predicate(node, node, edge)
{
    using graph_type = compiled_adjacency_list<Node, Edge>;
    using map_entry = astar_search_entry<Vertex, Distance>;
    struct queue_entry {
        Distance priority;
        Vertex   vertex;
    };

    std::unordered_map<Vertex, map_entry, map_iterator_hasher<Vertex>> search_map;
    std::priority_queue frontier = [] {
        return std::priority_queue(
            [] (const queue_entry& a, const queue_entry& b) { return a.priority > b.priority; },
            std::vector<queue_entry>());
    }();

    auto push = [heuristic, &goal, &search_map, &frontier]
                (Vertex   predecessor,
                 Vertex   vertex,
                 Distance distance) {
        bool opened = false;

        if (auto it = search_map.find(vertex); it == search_map.end())
            search_map.emplace(vertex, map_entry{predecessor, distance, opened = true});
        else if (distance < it->second.distance)
            it->second = {predecessor, distance, opened = true};

        if (opened)
            frontier.push({distance + heuristic(vertex->first, goal->first), vertex});
    };

    push(start, start, static_cast<Distance>(0));
    while (!frontier.empty()) {
        const Vertex vertex = frontier.top().vertex;
        const auto& [node, edges] = *vertex;
        const map_entry& entry = search_map.at(vertex);
        frontier.pop();

        if (!entry.open)
            continue;

        const Distance distance = entry.distance;
        if (!visitor(entry.predecessor->first, node) || vertex == goal)
            break;

        for (const typename graph_type::edge_type& edge : edges)
            if (edge_predicate(edge.source->first, edge.target->first, edge))
                push(edge.source, edge.target, distance + edge->distance);
    }

    return search_map;
}

template<class Vertex, class Distance, class... MapArgs>
std::optional<std::vector<Vertex>>
get_path(const Vertex& start,
         const Vertex& goal,
         const std::unordered_map<Vertex, astar_search_entry<Vertex, Distance>,
                                  MapArgs...>& search_map)
{
    auto it = search_map.find(goal);
    auto end = search_map.find(start);

    if (it == search_map.end() || end == search_map.end())
        return std::nullopt;

    std::vector<Vertex> path;
    for (; it != end; it = search_map.find(it->second.predecessor))
        path.push_back(it->first);
    std::reverse(path.begin(), path.end());

    return std::make_optional(path);
}

} // namespace simulacrum
