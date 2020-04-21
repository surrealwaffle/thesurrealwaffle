
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__TAGS__COLLISION_BSP_HPP
#define SENTINEL__TAGS__COLLISION_BSP_HPP

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

    struct collision_bsp {
        struct bsp3d_node; ///< The node type in the BSP tree.

        struct plane;   ///< Represents an affine, 3d hyperplane.

        struct leaf;
        struct bsp2d_reference;
        struct bsp2d_node;

        struct surface; ///< Represents a collidable surface (an n-gon) in the collision BSP.
        struct edge;    ///< Represents an edge of a collidable surface in the collision BSP, with adjacency information for following edges of a surface.
        struct vertex;  ///< The vertex type for surfaces in the collision BSP.

        tag_block<bsp3d_node> bsp3d_nodes;
        tag_block<plane>      planes;
        tag_block<leaf>       leaves;

        tag_block<bsp2d_reference> bsp2d_references;
        tag_block<bsp2d_node>      bsp2d_nodes;

        tag_block<surface> surfaces;
        tag_block<edge>    edges;
        tag_block<vertex>  vertices;
    }; static_assert(sizeof(collision_bsp) == 0x60);

    struct collision_bsp::bsp3d_node {
        /** \brief The index of the plane the node partitions.
         *
         * If this value is negative, the tested
         * normal should be anti-parallel to the plane normal.
         *
         * To retrieve the actual index, use #plane_index().
         * To check if the effective normal should negated, use #is_plane_reversed().
         */
        int32 plane;

        /** \brief The indices of the children nodes partitioned by the plane.
         *
         * If a child index is negative, then the index refers to a leaf,
         * not a \ref bsp3d_node.
         *
         * Presumably, an index of `-1` refers to no child.
         */
        int32 children[2];
                           // {back_child, front_child}

        /** \brief Returns the index of the partitioning plane. */
        constexpr int32 plane_index() const noexcept { return plane & 0x7FFFFFFF; }

        /** \brief Returns `true` if the tested normal is anti-parallel to the plane normal, otherwise `false`. */
        constexpr bool is_plane_reversed() const noexcept { return plane < 0; }
    }; static_assert(sizeof(collision_bsp::bsp3d_node) == 0x0C);

    struct collision_bsp::plane {
        direction3d normal; ///< The normal parameter in point-normal form.
        real        d;      ///< The scalar parameter in point-normal form.
    }; static_assert(sizeof(collision_bsp::plane) == 0x10);

    struct collision_bsp::leaf {
        int16 flags;
        int16 bsp2d_reference_count;
        int32 first_bsp2d_reference;
    }; static_assert(sizeof(collision_bsp::leaf) == 0x08);

    struct collision_bsp::bsp2d_reference {
        int32 plane;
        int32 bsp2d_node;
    }; static_assert(sizeof(collision_bsp::bsp2d_reference) == 0x08);

    struct collision_bsp::bsp2d_node {
        struct plane {
            real normal[2];
            real d;
        } plane;

        int32 children[2];
                           // {left_child, right_child}
    }; static_assert(sizeof(collision_bsp::bsp2d_node) == 0x14);

    struct collision_bsp::surface {
        /** \brief Defines bitmasks used to determine surface properties.
         *
         * Use bitwise operators to test the properties,
         * i.e. `flags & TWO_SIDED`.
         *
         * Surfaces marked as two sided are implemented
         * with a separate surface that marks the other side.
         */
        enum FLAGS : int8;

        /** \brief The index of the plane the node partitions.
         *
         * If this value is negative, the tested
         * normal should be anti-parallel to the plane normal.
         *
         * To retrieve the actual index, use #plane_index().
         * To check if the effective normal should negated, use #is_plane_reversed().
         */
        int32 plane;

        int32       first_edge;   ///< The index of the first forward edge of this surface.
        flags_byte  flags;        ///< The properties of this surfaces, see \ref FLAGS.
        index_byte  breakable_surface; ///< If this surface is breakable, this indexes into structure BSP `breakable_surfaces` tag block.
        index_short material;          ///< The index of the collision material for this surface, or `-1` if there is no associated material.

        /** \brief Tests if all the bits set by \a mask are set.
         *
         * \return `true` if all the bits in \a mask are set, otherwise `false`.
         */
        constexpr bool test_flags(int8 const mask) const noexcept { return mask == (flags & mask); }

        /** \brief Tests if \a property is set.
         *
         * \return `true` if \a property is set, otherwise `false`.
         */
        constexpr bool test_flags(FLAGS const property) const noexcept { return flags & property; }

        /** \brief Returns the index of the plane this surface lies on. */
        constexpr int32 plane_index() const noexcept { return plane & 0x7FFFFFFF; }

        /** \brief Returns `true` if the surface's normal is anti-parallel to the plane normal, otherwise `false`. */
        constexpr bool is_reversed() const noexcept { return plane < 0; }
    }; static_assert(sizeof(collision_bsp::surface) == 0x0C);

    enum collision_bsp::surface::FLAGS : int8 {
        TWO_SIDED = 1, ///< The surface is two sided.
        INVISIBLE = 2, ///< The surface is invisible.
        CLIMBABLE = 4, ///< The surface is climbable.
        BREAKABLE = 8  ///< The surface is breakable.
    };

    struct collision_bsp::edge {
        /** \brief The vertex indices of vertices defining this edge.
         *
         * If a surface index is `surface[i]`, for `i` less than 2,
         * then `vertices[i]` and `vertices[1-i]` are the start
         * and end vertices for that edge of the surface, respectively.
         */
        int32 vertices[2];

        /** \brief The edge indices of edges adjacent to this edge.
         *
         * If a surface index is `surface[i]` for `i` less than `2`,
         * then the next edge defining the surface is `edges[i]`.
         */
        int32 edges[2];

        /** \brief The surface indices of surfaces on both sides of this edge.
         *
         * These indices are used to determine which edge to follow in #edges
         * and which indices correspond to the start and end vertices in #vertices.
         */
        int32 surfaces[2];

        /** \brief Returns the start vertex on this edge for \a surface.
         *
         * Assumes \a surface is either `surfaces[0]` or `surfaces[1]`.
         */
        int32 start_vertex(int32 const surface) const noexcept { return vertices[surface == surfaces[1]]; }

        /** \brief Returns the end vertex on this edge for \a surface.
         *
         * Assumes \a surface is either `surfaces[0]` or `surfaces[1]`.
         */
        int32 end_vertex(int32 const surface) const noexcept { return vertices[surface != surfaces[1]]; }

        /** \brief Returns the next edge to follow for \a surface.
         *
         * Assumes \a surface is either `surfaces[0]` or `surfaces[1]`.
         */
        int32 next_edge(int32 const surface) const noexcept { return edges[surface == surfaces[1]]; }

        /** \brief Returns the surface on the other side of the edge.
         *
         * Assumes \a surface is either `surfaces[0]` or `surfaces[1]`.
         */
        int32 adjacent_surface(int32 const surface) const noexcept { return surfaces[surface == surfaces[0]]; }
    }; static_assert(sizeof(collision_bsp::edge) == 0x18);

    struct collision_bsp::vertex {
        /** \brief The position of this vertex. */
        position3d point;

        int32 first_edge;
    }; static_assert(sizeof(collision_bsp::vertex) == 0x10);

} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__COLLISION_BSP_HPP
