#ifndef SIMULA__NAV__CBSPGRAPH_HPP
#define SIMULA__NAV__CBSPGRAPH_HPP

#include <cstddef>

#include <functional>
#include <vector>
#include <unordered_map>
#include <ostream>

#include <sentinel/tags/collision_bsp.hpp>

namespace simula::nav {

using real3 = sentinel::real3;

/** \brief A delegate for a surface or edge in a `sentinel::tags::collision_bsp`. */
struct CBSPElement;

/** \brief Represents a surface or edge in a \ref CBSPGraph. */
struct CBSPGraphVertex;

/** \brief A graph where vertices are surfaces and edges in a `sentinel::tags::collision_bsp`.
 *
 * Graphs of this kind are associated with at most one collision BSP.
 *
 * The neighbors of a graph vertex depends on the collision BSP element type.
 * Only neighbors that pass the filter are included in the neighbors:
 *  * If a vertex represents a collision BSP edge, its neighbors are joined surfaces and (optionally) the edges of those surfaces.
 *  * If a vertex represents a collision BSP surface, its neighbors are the surface polygon edges.
 */
class CBSPGraph;

struct CBSPElement {
    enum TYPE {
        EDGE,   ///< Indicates the element is an edge.
        SURFACE ///< Indicates the element is a surface.
    };

    TYPE type;  ///< The element type in the BSP.
    long index; ///< The element index in the BSP.

    /** \brief Returns `true` if and only if this element and \a other represent the same element in the collision BSP.
     *
     * Assumes that this element and \a other belong to the same collision BSP.
     */
    constexpr bool operator==(CBSPElement const& other) const noexcept {
        return (type == other.type) & (index == other.index);
    }

    /** \brief Returns `true` if and only if this element and \a other represent different elements in the collision BSP.
     *
     * Assumes that this element and \a other belong to the same collision BSP.
     */
    constexpr bool operator!=(CBSPElement const& other) const noexcept {
        return (type != other.type) | (index != other.index);
    }

    /** \brief Returns the represented element as a collision BSP surface.
     *
     * The effect of this function is undefined if the referred element is not a surface.
     */
    sentinel::tags::collision_bsp::surface const& as_surface(sentinel::tags::collision_bsp const& collision_bsp) const noexcept {
        return collision_bsp.surfaces[index];
    }

    /** \brief Returns the represented element as a collision BSP edge.
     *
     * The effect of this function is undefined if the referred element is not an edge.
     */
    sentinel::tags::collision_bsp::edge const&    as_edge(sentinel::tags::collision_bsp const& collision_bsp)    const noexcept {
        return collision_bsp.edges[index];
    }
};

} // namespace simula::nav

namespace std {
    // add specialization to std::hash for simula::nav::CBSPElement
    template<>
    struct hash<simula::nav::CBSPElement> {
        using argument_type = simula::nav::CBSPElement;
        using result_type = typename std::hash<decltype(argument_type::index)>::result_type;

        result_type operator()(argument_type const& e) const noexcept {
            return std::hash<decltype(e.index)>{}(e.index);
        }
    };
} // namespace std

namespace simula::nav {

struct CBSPGraphVertex {
    using reference_wrapper = std::reference_wrapper<CBSPGraphVertex>;

    std::size_t graph_index; ///< The index of this node in the \ref CBSPGraph.
    CBSPElement element;     ///< The BSP element properties.
    std::vector<reference_wrapper> neighbors; ///< The neighbors of this vertex.
    CBSPGraph                      *graph;    ///< A pointer to the containing graph.

    /** \brief Returns `true` if and only if this vertex represents a surface in the collision BSP. */
    bool is_surface() const noexcept { return element.type == element.TYPE::SURFACE; }

    /** \brief Returns `true` if and only if this vertex represents an edge in the collision BSP. */
    bool is_edge()    const noexcept { return element.type == element.TYPE::EDGE; }

    /** \brief Returns the represented element as a collision BSP surface.
     *
     * The effect of this function is undefined if the referred element is not a surface.
     */
    sentinel::tags::collision_bsp::surface const& as_surface() const noexcept;

    /** \brief Returns the represented element as a collision BSP edge.
     *
     * The effect of this function is undefined if the referred element is not an edge.
     */
    sentinel::tags::collision_bsp::edge const&    as_edge()    const noexcept;

    /** \brief Returns the average point over the vertices of the represented element. */
    sentinel::real3 centroid() const noexcept;

    /** \brief Applies function object \a f to every collision BSP edge of the represented element.
     *
     * \a f is supplied first the edge index and then the edge itself.
     *
     * If this vertex refers to an edge, \a f applies only to it.
     * If this vertex refers to a surface, \a f applies only to the edges of the polygon.
     *
     * If this vertex refers to a surface, the edges counter-clockwise about the surface normal.
     *
     * \return `std::move(f)`
     */
    template<class BinaryFunction>
    BinaryFunction for_each_edge(BinaryFunction f) const;

    /** \brief Applies function object \a f to every collision BSP vertex on the represented element exactly once.
     *
     * \a f is supplied first the vertex index and then the edge itself.
     *
     * \return `std::move(f)`
     */
    template<class BinaryFunction>
    BinaryFunction for_each_vertex(BinaryFunction f) const;
};

class CBSPGraph {
public:
    /** \brief The collision BSP type. */
    using collision_bsp_type = sentinel::tags::collision_bsp;

    /** \brief The graph vertex type. */
    using vertex_type = CBSPGraphVertex;

    /** \brief The storage type for the graph vertices. */
    using vertex_container_type = std::vector<vertex_type>;

    /** \brief The mapping type from collision BSP elements to graph vertex indices. */
    using bsp_element_map_type  = std::unordered_map<CBSPElement, vertex_container_type::size_type>;

    /** \brief Returns `true` if and only if the graph is constructed. */
    bool is_constructed() const noexcept { return is_constructed_; }

    /** \brief Returns a reference to collision BSP associated with this graph.
     *
     * The result and effects of this function are undefined
     * if the graph is not constructed.
     */
    collision_bsp_type const& collision_bsp() const noexcept { return *collision_bsp_; }

    /** \brief Empties the graph. */
    void clear() { *this = CBSPGraph(); }

    /** \brief Constructs the collision BSP graph from \a collision_bsp and filtering elements with \a filter.
     *
     * If this function returns `false`, the graph is cleared via #clear().
     *
     * \param[in] collision_bsp The collision BSP to construct the graph for.
     * \param[in] connect_edge_edge_neighbors Supply as `true` to have collision BSP edges connected to adjacent edges in the graph.
     * \param[in] filter A functional that returns `true` only for collision BSP elements that should be included in the graph.
     * \return `true` if the graph was constructed, otherwise `false`.
     */
    bool build(collision_bsp_type const* collision_bsp, bool connect_edge_edge_neighbors, std::function<bool(CBSPElement const&)> filter = [] (auto&&) { return true; });

    /** \brief Returns the graph vertex container. */
    vertex_container_type const& vertices() const noexcept { return vertices_; }

    /** \brief Returns the mapping from collision BSP elements to indices into the graph vertex container. */
    bsp_element_map_type const& element_map() const noexcept { return element_map_;}

    /** \brief Writes the graph to an output stream.
     *
     * \param[in] ostream The stream to write to.
     * \return `true` if the graph was completely written to \a ostream, otherwise `false`.
     */
    bool export_as_obj(std::ostream& ostream) const;

private:
    bool                     is_constructed_ = false;   ///< Indicates if the graph was constructed.
    collision_bsp_type const *collision_bsp_ = nullptr; ///< The collision BSP associated with the graph.

    vertex_container_type vertices_;    ///< The graph vertices.
    bsp_element_map_type  element_map_; ///< The mapping from BSP elements to graph vertex indices.
};

template<class BinaryFunction>
BinaryFunction CBSPGraphVertex::for_each_edge(BinaryFunction f) const {
    using namespace sentinel;
    CBSPGraph::collision_bsp_type const& collision_bsp = graph->collision_bsp();

    switch (element.type) {
    case element.TYPE::EDGE:
        f(element.index, collision_bsp.edges[element.index]);
        break;
    case element.TYPE::SURFACE:
        {
            tags::collision_bsp::surface const& surface = collision_bsp.surfaces[element.index];
            auto const first_edge = surface.first_edge;
            auto edge_index = first_edge;
            do {
                tags::collision_bsp::edge const& edge = collision_bsp.edges[edge_index];
                f(edge_index, edge);
                edge_index = edge.next_edge(element.index);
            } while(edge_index != first_edge);
        }
        break;
    }

    return std::move(f);
}

template<class BinaryFunction>
BinaryFunction CBSPGraphVertex::for_each_vertex(BinaryFunction f) const {
    using namespace sentinel;
    CBSPGraph::collision_bsp_type const& collision_bsp = graph->collision_bsp();

    switch(element.type) {
    case element.TYPE::EDGE:
        {
            tags::collision_bsp::edge const& edge = collision_bsp.edges[element.index];
            for (auto const vertex_index : edge.vertices)
                f(vertex_index, collision_bsp.vertices[vertex_index]);
        }
        break;
    case element.TYPE::SURFACE:
        {
            auto op = [&f, &collision_bsp, index = element.index] (auto&&, tags::collision_bsp::edge const& edge) {
                auto const start_vertex = edge.start_vertex(index);
                f(start_vertex, collision_bsp.vertices[start_vertex]);
            };
            this->for_each_edge(op);
        }
        break;
    }

    return std::move(f);
}

inline sentinel::tags::collision_bsp::surface const& CBSPGraphVertex::as_surface() const noexcept {
    return element.as_surface(graph->collision_bsp());
}

inline sentinel::tags::collision_bsp::edge const& CBSPGraphVertex::as_edge() const noexcept {
    return element.as_edge(graph->collision_bsp());
}

} // namespace simula::nav

#endif // SIMULA__NAV__CBSP_GRAPH_HPP
