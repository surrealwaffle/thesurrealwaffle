
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "bsp_interface.hpp"

#include <iterator>
#include <memory>

namespace simulacrum { namespace utility {

interface_cbsp_surface::interface_cbsp_surface(const interface_cbsp& collision_bsp,
                                               long surface_index)
    : cbsp_elem_base{collision_bsp,
                     surface_index,
                     collision_bsp->surfaces[surface_index]} { }

interface_cbsp_edge::interface_cbsp_edge(const interface_cbsp& collision_bsp,
                                         long edge_index)
    : cbsp_elem_base{collision_bsp,
                     edge_index,
                     collision_bsp->edges[edge_index]} { }

interface_cbsp_vertex::interface_cbsp_vertex(const interface_cbsp& collision_bsp,
                                             long vertex_index)
    : cbsp_elem_base{collision_bsp,
                     vertex_index,
                     collision_bsp->vertices[vertex_index]} { }

interface_cbsp_surface
interface_cbsp::make_surface_interface(long surface_index) const
{
    return interface_cbsp_surface(*this, surface_index);
}

interface_cbsp_edge
interface_cbsp::make_edge_interface(long edge_index) const
{
    return interface_cbsp_edge(*this, edge_index);
}

interface_cbsp_vertex
interface_cbsp::make_vertex_interface(long vertex_index) const
{
    return interface_cbsp_vertex(*this, vertex_index);
}

sentinel::tags::collision_bsp::plane
interface_cbsp_surface::plane() const
{
    auto p = coll.get()->planes[get().plane_index()];

    if (get().is_reversed())
        p.normal = -p.normal;

    return p;
}

interface_cbsp_surface::surface_iterator
interface_cbsp_surface::surfaces_begin() const
{
    return {cbsp_edge_index_iterator{1,
                                     *this,
                                     first_edge()},
            surface_interfacer{&coll.get()}};
}

interface_cbsp_surface::surface_iterator
interface_cbsp_surface::surfaces_end() const
{
    return {cbsp_edge_index_iterator{0,
                                     *this,
                                     first_edge()},
            surface_interfacer{&coll.get()}};
}

interface_cbsp_surface::edge_iterator
interface_cbsp_surface::edges_begin() const
{
    return {cbsp_edge_index_iterator{1,
                                     *this,
                                     first_edge()},
            edge_interfacer{&coll.get()}};
}

interface_cbsp_surface::edge_iterator
interface_cbsp_surface::edges_end() const
{
    return {cbsp_edge_index_iterator{0,
                                     *this,
                                     first_edge()},
            edge_interfacer{&coll.get()}};
}

interface_cbsp_surface::vertex_iterator
interface_cbsp_surface::vertices_begin() const
{
    return {cbsp_edge_index_iterator{1,
                                     *this,
                                     first_edge()},
            vertex_interfacer{&coll.get()}};
}

interface_cbsp_surface::vertex_iterator
interface_cbsp_surface::vertices_end() const
{
    return {cbsp_edge_index_iterator{0,
                                     *this,
                                     first_edge()},
            vertex_interfacer{&coll.get()}};
}

rough_span<interface_cbsp_surface::surface_iterator>
interface_cbsp_surface::surfaces() const
{
    return {surfaces_begin(), surfaces_end()};
}

rough_span<boost::transform_iterator<edge_interfacer, cbsp_edge_index_iterator>>
interface_cbsp_surface::edges() const
{
    return {edges_begin(), edges_end()};
}

rough_span<boost::transform_iterator<vertex_interfacer, cbsp_edge_index_iterator>>
interface_cbsp_surface::vertices() const
{
    return {vertices_begin(), vertices_end()};
}

sentinel::real
interface_cbsp_surface::area() const
{
    const auto first_point = vertices_begin()->point();

    sentinel::real result = 0.0f;
    auto previous_point = first_point;

    for (const auto& vertex : vertices())
    {
        const auto point = vertex.point();
        result += 0.5f * norm(cross(previous_point - first_point,
                                    point - first_point));
        previous_point = point;
    }

    return result;
}

sentinel::real3d
interface_cbsp_surface::centroid() const
{
    const auto first_point = vertices_begin()->point();

    sentinel::real3d result = sentinel::real3d::zero;
    auto previous_point = first_point;

    for (const auto& vertex : vertices())
    {
        const auto point    = vertex.point();
        const auto tri_area = 0.5f * norm(cross(previous_point - first_point,
                                                point - first_point));
        result += (1.0f / 3.0f) * tri_area * (first_point + previous_point + point);
        previous_point = point;
    }

    result *= (1 / area());
    return result;
}

long
interface_cbsp::next_vertex_index(long surface_index, long edge_index) const
{
    const auto& edge = collision_bsp.get().edges[edge_index];
    return edge.vertices[edge.surfaces[0] != surface_index ? 1 : 0];
}

long interface_cbsp::next_edge_index(long surface_index, long edge_index) const
{
    const auto& edge = collision_bsp.get().edges[edge_index];
    return edge.edges[edge.surfaces[0] != surface_index ? 1 : 0];
}

long interface_cbsp::next_surface_index(long surface_index, long edge_index) const
{
    const auto& edge = collision_bsp.get().edges[edge_index];
    return edge.surfaces[edge.surfaces[0] == surface_index ? 1 : 0];
}

cbsp_edge_index_iterator::cbsp_edge_index_iterator(int cycle,
                                                   const interface_cbsp_surface& surface,
                                                   const interface_cbsp_edge&    edge) noexcept
    : cycle_(cycle)
    , value_(std::addressof(surface), edge.index)
    , first_edge_index_(this->value_.second) { }

bool
cbsp_edge_index_iterator::operator==(const cbsp_edge_index_iterator& other) const noexcept
{
    return cycle_ == other.cycle_ && value_.second == other.value_.second;
}

bool
cbsp_edge_index_iterator::operator!=(const cbsp_edge_index_iterator& other) const noexcept
{
    return !(*this == other);
}

cbsp_edge_index_iterator&
cbsp_edge_index_iterator::operator++() { return (increment(), *this); }

cbsp_edge_index_iterator
cbsp_edge_index_iterator::operator++(int)
{
    auto copy = *this;
    (void)(++(*this));
    return copy;
}

cbsp_edge_index_iterator::reference
cbsp_edge_index_iterator::operator*() const noexcept
{
    return value_;
}

cbsp_edge_index_iterator::pointer
cbsp_edge_index_iterator::operator->() const noexcept
{
    return std::addressof(value_);
}

void
cbsp_edge_index_iterator::increment()
{
    edge_index() = surface().coll.get().next_edge_index(surface().index, edge_index());
    if (edge_index() == first_edge_index_)
        --cycle_;
}

interface_cbsp_surface
surface_interfacer::operator()(long index) const
{
    return cbsp->make_surface_interface(index);
}

interface_cbsp_surface
surface_interfacer::operator()(const std::pair<const interface_cbsp_surface*, long>& p) const
{
    return cbsp->make_surface_interface(cbsp->next_surface_index(p.first->index,
                                                                 p.second));
}

interface_cbsp_edge
edge_interfacer::operator()(long index) const
{
    return cbsp->make_edge_interface(index);
}

interface_cbsp_edge
edge_interfacer::operator()(const std::pair<const interface_cbsp_surface*, long>& p) const
{
    return cbsp->make_edge_interface(cbsp->next_edge_index(p.first->index,
                                                           p.second));
}

interface_cbsp_vertex
vertex_interfacer::operator()(long index) const
{
    return cbsp->make_vertex_interface(index);
}

interface_cbsp_vertex
vertex_interfacer::operator()(const std::pair<const interface_cbsp_surface*, long>& p) const
{
    return cbsp->make_vertex_interface(cbsp->next_vertex_index(p.first->index,
                                                               p.second));
}

interface_cbsp_edge
interface_cbsp_surface::first_edge() const
{
    return coll.get().make_edge_interface(get().first_edge);
}

std::array<interface_cbsp_vertex, 2>
interface_cbsp_edge::vertices() const
{
    return {
        coll.get().make_vertex_interface(get().vertices[0]),
        coll.get().make_vertex_interface(get().vertices[1])
    };
}

std::array<interface_cbsp_edge, 2>
interface_cbsp_edge::edges() const
{
    return {
        coll.get().make_edge_interface(get().edges[0]),
        coll.get().make_edge_interface(get().edges[1])
    };
}

std::array<interface_cbsp_surface, 2>
interface_cbsp_edge::surfaces() const
{
    return {
        coll.get().make_surface_interface(get().surfaces[0]),
        coll.get().make_surface_interface(get().surfaces[1])
    };
}

interface_cbsp_vertex
interface_cbsp_edge::next_vertex(const interface_cbsp_surface& surface) const
{
    return coll.get().make_vertex_interface(coll.get().next_vertex_index(surface.index, index));
}

interface_cbsp_edge
interface_cbsp_edge::next_edge(const interface_cbsp_surface& surface) const
{
    return coll.get().make_edge_interface(coll.get().next_edge_index(surface.index, index));
}

interface_cbsp_surface
interface_cbsp_edge::next_surface(const interface_cbsp_surface& surface) const
{
    return coll.get().make_surface_interface(coll.get().next_surface_index(surface.index, index));
}

sentinel::real3d
interface_cbsp_edge::centroid() const
{
    const auto& vertices = this->vertices();
    return 0.5f * (vertices[0].point() + vertices[1].point());
}

sentinel::position3d&
interface_cbsp_vertex::point() const
{
    return get().point;
}

interface_cbsp_edge
interface_cbsp_vertex::first_edge() const
{
    return coll.get().make_edge_interface(get().first_edge);
}

} } // namespace simulacrum::utility
