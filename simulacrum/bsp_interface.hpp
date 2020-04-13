#pragma once

#include <cstddef>

#include <functional>
#include <iterator>
#include <utility>

#include <boost/iterator/transform_iterator.hpp>

#include <sentutil/all.hpp>

#include "utility.hpp"

namespace simulacrum { namespace utility {

struct interface_cbsp;
struct interface_cbsp_surface;
struct interface_cbsp_edge;
struct interface_cbsp_vertex;

class cbsp_edge_index_iterator
{
public:
    using difference_type = std::ptrdiff_t;
    using value_type      = const std::pair<const interface_cbsp_surface*, long>;
    using pointer         = value_type*;
    using reference       = value_type&;
    using iterator_category = std::forward_iterator_tag;

    cbsp_edge_index_iterator() = default;

    cbsp_edge_index_iterator(int cycle,
                             const interface_cbsp_surface& surface,
                             const interface_cbsp_edge&    edge) noexcept;

    bool operator==(const cbsp_edge_index_iterator& other) const noexcept;

    bool operator!=(const cbsp_edge_index_iterator& other) const noexcept;

    cbsp_edge_index_iterator& operator++();

    cbsp_edge_index_iterator operator++(int);

    reference operator*()  const noexcept;
    pointer   operator->() const noexcept;

private:
    int                                            cycle_;
    std::pair<const interface_cbsp_surface*, long> value_;
    long                                           first_edge_index_;

    const interface_cbsp_surface& surface() const noexcept { return *(value_.first); }
    long&       edge_index()       noexcept { return value_.second; }
    const long& edge_index() const noexcept { return value_.second; }

    void increment();
};

struct surface_interfacer {
    const interface_cbsp* cbsp;

    interface_cbsp_surface
    operator()(long index) const;

    interface_cbsp_surface
    operator()(const std::pair<const interface_cbsp_surface*, long>&) const;
};

struct edge_interfacer {
    const interface_cbsp* cbsp;

    interface_cbsp_edge
    operator()(long index) const;

    interface_cbsp_edge
    operator()(const std::pair<const interface_cbsp_surface*, long>&) const;
};

struct vertex_interfacer {
    const interface_cbsp* cbsp;

    interface_cbsp_vertex
    operator()(long index) const;

    interface_cbsp_vertex
    operator()(const std::pair<const interface_cbsp_surface*, long>&) const;
};

template<class WrappedType>
struct cbsp_elem_base {
    std::reference_wrapper<const interface_cbsp> coll;
    long                                         index;
    std::reference_wrapper<WrappedType>          reference;

    WrappedType& get() const { return reference.get(); }
    WrappedType* operator->() const { return &reference.get(); }
    bool operator==(const cbsp_elem_base& other) const { return index == other.index; }
    bool operator!=(const cbsp_elem_base& other) const { return !(*this == other); }
};

struct interface_cbsp_vertex
    : cbsp_elem_base<sentinel::tags::collision_bsp::vertex>
{
    interface_cbsp_vertex(const interface_cbsp& collision_bsp,
                          long vertex_index);

    sentinel::position3d& point()      const;
    interface_cbsp_edge   first_edge() const;
};

struct interface_cbsp_edge
    : cbsp_elem_base<sentinel::tags::collision_bsp::edge>
{
    interface_cbsp_edge(const interface_cbsp& collision_bsp,
                        long edge_index);

    std::array<interface_cbsp_vertex, 2>  vertices() const;
    std::array<interface_cbsp_edge, 2>    edges()    const;
    std::array<interface_cbsp_surface, 2> surfaces() const;

    interface_cbsp_vertex  next_vertex(const interface_cbsp_surface& surface)  const;
    interface_cbsp_edge    next_edge(const interface_cbsp_surface& surface)    const;
    interface_cbsp_surface next_surface(const interface_cbsp_surface& surface) const;

    sentinel::real3d centroid() const;
};

struct interface_cbsp_surface
    : cbsp_elem_base<sentinel::tags::collision_bsp::surface>
{
    using surface_iterator = boost::transform_iterator<surface_interfacer, cbsp_edge_index_iterator>;
    using edge_iterator    = boost::transform_iterator<edge_interfacer, cbsp_edge_index_iterator>;
    using vertex_iterator  = boost::transform_iterator<vertex_interfacer, cbsp_edge_index_iterator>;

    interface_cbsp_surface(const interface_cbsp& collision_bsp,
                           long surface_index);

    sentinel::tags::collision_bsp::plane plane()      const;
    interface_cbsp_edge                  first_edge() const;

    surface_iterator surfaces_begin() const;
    surface_iterator surfaces_end()   const;

    edge_iterator edges_begin() const;
    edge_iterator edges_end()   const;

    vertex_iterator vertices_begin() const;
    vertex_iterator vertices_end()   const;

    rough_span<surface_iterator> surfaces() const;
    rough_span<edge_iterator>    edges() const;
    rough_span<vertex_iterator>  vertices() const;

    sentinel::real   area()     const;
    sentinel::real3d centroid() const;
};

struct interface_cbsp
{
    using surface_iterator = boost::transform_iterator<surface_interfacer, boost::counting_iterator<long>>;
    using edge_iterator    = boost::transform_iterator<edge_interfacer, boost::counting_iterator<long>>;
    using vertex_iterator  = boost::transform_iterator<vertex_interfacer, boost::counting_iterator<long>>;

    std::reference_wrapper<sentinel::tags::collision_bsp> collision_bsp;

    rough_span<surface_iterator> surfaces
        = make_rough_counting_span(0L, (long)collision_bsp.get().surfaces.count,
                                   surface_interfacer{this});

    rough_span<edge_iterator> edges
        = make_rough_counting_span(0L, (long)collision_bsp.get().edges.count,
                                   edge_interfacer{this});

    rough_span<vertex_iterator> vertices
        = make_rough_counting_span(0L, (long)collision_bsp.get().vertices.count,
                                   vertex_interfacer{this});

    sentinel::tags::collision_bsp* operator->() const { return &collision_bsp.get(); }

    interface_cbsp_surface make_surface_interface(long surface_index) const;
    interface_cbsp_edge    make_edge_interface(long edge_index)       const;
    interface_cbsp_vertex  make_vertex_interface(long vertex_index)   const;

    long next_surface_index(long surface_index, long edge_index) const;
    long next_edge_index(long surface_index, long edge_index) const;
    long next_vertex_index(long surface_index, long edge_index) const;
};

} } // namespae simulacrum::utility
