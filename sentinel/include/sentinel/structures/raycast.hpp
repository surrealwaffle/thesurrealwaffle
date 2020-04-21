
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__STRUCTURES__RAYCAST_HPP
#define SENTINEL__STRUCTURES__RAYCAST_HPP

#include <sentinel/types.hpp>
#include <sentinel/fwd/table_fwd.hpp>

namespace sentinel {

struct raycast_result_type {
    int16  hit_type; ///< `-1` for none, `0` for water, `2` for BSP, `3` for object

    int32  unknown00;
    int16  unknown01;
    int32  unknown02;
    int16  unknown03;

    real   portion_to_target; ///< Equal to `norm(terminal-source)/norm(target-source)`
    real3d terminal;          ///< The point of intersection or the endpoint.

    // if hit_type == none, plane and material are not set

    struct {
        direction3d normal;
        real  d;
    } plane; ///< The plane equation for the intersected plane.

    int16 material_type; ///< The type of the material as set in the shader of the surface hit.

    // if hit_type != object, no fields in [x38 .. x44) are set

    identity<object> hit_identity;      ///< The identity of the object hit by the raycast, if #hit_type is `3`.

    int16    object_region_index;        ///< The index of the collision model region intersected.
    int16    object_node_index;          ///< The index of the collision model node intersected.
    int16    object_collision_bsp_index; ///< The index of the collision BSP intersected under #object_node.

    // if anything was hit, the fields below are set
    // related to the surface hit in the collision bsp

    int32    surface_index;
    int32    plane_index;
    int8     flags;
    int8     breakable_surface;
    int16    material_index;

    /** Returns `true` if the raycast hit a surface, otherwise `false`. */
    constexpr operator bool() const noexcept { return hit_type != -1; }
}; static_assert(sizeof(raycast_result_type) == 0x50);

static_assert(offsetof(raycast_result_type, hit_type) == 0x00);
static_assert(offsetof(raycast_result_type, portion_to_target) == 0x14);
static_assert(offsetof(raycast_result_type, terminal) == 0x18);
static_assert(offsetof(raycast_result_type, plane) == 0x24);
static_assert(offsetof(raycast_result_type, material_type) == 0x34);

static_assert(offsetof(raycast_result_type, hit_identity) == 0x38);
static_assert(offsetof(raycast_result_type, object_region_index) == 0x3C);
static_assert(offsetof(raycast_result_type, object_node_index) == 0x3E);
static_assert(offsetof(raycast_result_type, object_collision_bsp_index) == 0x40);

static_assert(offsetof(raycast_result_type, surface_index) == 0x44);
static_assert(offsetof(raycast_result_type, plane_index) == 0x48);
static_assert(offsetof(raycast_result_type, flags) == 0x4C);
static_assert(offsetof(raycast_result_type, breakable_surface) == 0x4D);
static_assert(offsetof(raycast_result_type, material_index) == 0x4E);

}

#endif // SENTINEL__STRUCTURES__RAYCAST_HPP
