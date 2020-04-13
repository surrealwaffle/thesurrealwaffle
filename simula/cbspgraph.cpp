#include "cbspgraph.hpp"
#include "vector_extensions.hpp"

namespace simula::nav {

sentinel::real3 CBSPGraphVertex::centroid() const noexcept {
    using sentinel::tags::collision_bsp;
    real3       point {0, 0, 0};
    std::size_t vertex_count = 0;
    auto sum_point = [&point, &vertex_count] (auto&&, collision_bsp::vertex const& vertex) {
        for (int i = 0; i < 3; ++i)
            point[i] += vertex.point[i];
        ++vertex_count;
    };

    for_each_vertex(sum_point);

    for (int i = 0; i < 3; ++i)
        point[i] /= vertex_count;

    return point;
}

bool CBSPGraph::build(collision_bsp_type const* collision_bsp, bool connect_edge_edge_neighbors, std::function<bool(CBSPElement const&)> filter) {
    using namespace sentinel;

    printf("clearing\n");
    clear();
    if (!collision_bsp)
        return false;

    // necessary for later, in step 2
    this->collision_bsp_ = collision_bsp;

    /* THE PLAN
       1. build the mapping of CBSPElements and vector of CBSPVertices, no adjacency information
       2. loop through the CBSP elements again and add in adjacency information
            only add CBSP elements as neighbors that are mapped in step #1
     */
    printf("reserving\n");
    { // reserve enough size of the vertices container
        using size_type = vertex_container_type::size_type;
        auto const number_graph_vertices = static_cast<size_type>(collision_bsp->edges.count)
                                         + static_cast<size_type>(collision_bsp->surfaces.count);
        this->vertices_.reserve(number_graph_vertices);
    }

    printf("step 1 for edges\n");
    { // STEP 1 for BSP EDGES
        long const number_edges = collision_bsp->edges.count;
        for (long i = 0; i < number_edges; ++i) {
            CBSPElement const element {CBSPElement::TYPE::EDGE, i};
            if (filter(element)) {
                CBSPGraphVertex const vertex  {vertices_.size(), element, {}, this};
                this->vertices_.push_back(vertex);
                this->element_map_[element] = vertex.graph_index;
            }
        }
    }

    printf("step 1 for surfaces\n");
    { // STEP 1 for BSP SURFACES
        long const number_surfaces = collision_bsp->surfaces.count;
        for (long i = 0; i < number_surfaces; ++i) {
            CBSPElement const element {CBSPElement::TYPE::SURFACE, i};
            if (filter(element)) {
                CBSPGraphVertex const vertex  {vertices_.size(), element, {}, this};
                this->vertices_.push_back(vertex);
                this->element_map_[element] = vertex.graph_index;
            }
        }
    }

    printf("step 2\n");
    { // STEP 2 - add adjacency information
        // assumes vertex is an edge, adds neighbor surfaces and their edges
        auto edge_op = [this, connect_edge_edge_neighbors] (vertex_type& vertex) {
            auto op = [this, &vertex] (long const edge_index, auto&&) {
                CBSPElement const edge_element {CBSPElement::EDGE, edge_index};
                if (vertex.element.index != edge_index && element_map_.count(edge_element))
                    vertex.neighbors.push_back(this->vertices_[element_map_.at(edge_element)]);
            };

            for (auto const surface_index : vertex.as_edge().surfaces) {
                CBSPElement const element {CBSPElement::SURFACE, surface_index};
                if (element_map_.count(element)) {
                    vertex_type& surface_vertex = this->vertices_[element_map_.at(element)];
                    vertex.neighbors.push_back(surface_vertex);
                }

                if (connect_edge_edge_neighbors) {
                    CBSPGraphVertex const surface_vertex {0, element, {}, this};
                    surface_vertex.for_each_edge(op);
                }
            }
        };
        // assumes vertex is a surface, adds neighbor polygon edges
        auto surface_op = [this] (vertex_type& vertex) {
            auto op = [&vertex, this] (long const edge_index, auto&&) {
                CBSPElement const element {CBSPElement::EDGE, edge_index};
                if (element_map_.count(element))
                    vertex.neighbors.push_back(this->vertices_[element_map_.at(element)]);
            };
            vertex.for_each_edge(op);
        };

        for (CBSPGraphVertex& vertex : this->vertices_) {
            switch(vertex.element.type) {
            case vertex.element.TYPE::EDGE: // add neighboring surfaces as neighbors
                edge_op(vertex);
                break;
            case vertex.element.TYPE::SURFACE: // add surface polygon edges as neighbors
                surface_op(vertex);
                break;
            }
            vertex.neighbors.shrink_to_fit();
        }
    }
    printf("step 2 done\n");

    this->is_constructed_ = true;
    this->collision_bsp_  = collision_bsp;

    bool const return_value = is_constructed();
    if (!return_value)
        this->clear();
    return return_value;
}

bool CBSPGraph::export_as_obj(std::ostream& ostream) const {
    if (!is_constructed())
        return false;

    ostream << "# CBSPGraph Export" << std::endl;

    // export element centroids
    ostream << std::endl << "# Collision BSP element centroids" << std::endl;

    for (auto&& vertex : vertices()) {
        constexpr float Z_NUDGE = 0.1;
        real3 const centroid = vertex.centroid();
        ostream << "v " << centroid[0] << " " << centroid[1] << " " << (centroid[2] + Z_NUDGE) << std::endl;
    }

    ostream << std::endl << "# Graph edges" << std::endl;
    for (auto&& vertex : vertices()) {
        for (auto&& neighbor : vertex.neighbors) {
            if (vertex.graph_index < neighbor.get().graph_index) {
                ostream << "f " << (vertex.graph_index + 1u) << " " << (neighbor.get().graph_index + 1u) << std::endl;
            }
        }
    }

    return true;
}

}
