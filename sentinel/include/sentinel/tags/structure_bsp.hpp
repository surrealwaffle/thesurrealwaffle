
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/collision_bsp.hpp>

namespace sentinel { namespace tags {

struct structure_bsp;

struct structure_bsp_header {
    structure_bsp* structure_bsp;

    int32 : 32;
    int32 : 32;
    int32 : 32;
    int32 : 32;

    union {
        int32 signature;
        char  usage[4];
    };
}; static_assert(sizeof(structure_bsp_header) == 0x18);

struct structure_bsp {
    struct leaf_block_element;

    tag_reference<void> lightmaps_bitmaps;
    real_bounds         vehicle_bounds;

    int32 unknown00[35]; // lighting

    tag_block<void>          collision_materials;
    tag_block<collision_bsp> collision_bsps;
    tag_block<void> nodes;

    real_bounds map_bounds[3]; // x, y, z

    tag_block<leaf_block_element> leaves;
    tag_block<void> leaf_surfaces;
    tag_block<void> surfaces;
    tag_block<void> lightmaps;

    int32 unknown01[3];

    // theres more here
    tag_block<void> lens_flares;
    tag_block<void> lens_flare_markers;
    tag_block<void> clusters;
    int32 unknown02[5];
    tag_block<void> cluster_portals;
    int32 unknown03[3];
    tag_block<void> breakable_surfaces;
    tag_block<void> fog_planes;
    tag_block<void> fog_regions;
    tag_block<void> fog_palette;
    int32 unknown04[6];
    tag_block<void> weather_palette;
    tag_block<void> weather_polyhedra;
    int32 unknown05[6];
    tag_block<void> pathfinding_surfaces;
    tag_block<void> pathfinding_edges;
    tag_block<void> background_sound_palette;
    tag_block<void> sound_environment_palette;
    int32 unknown06[11];
    tag_block<void> markers;
    tag_block<void> detail_objects;
    tag_block<void> runtime_decals;

    int32 unknown07[3];

    tag_block<void> leaf_map_leaves;
    tag_block<void> leaf_map_portals;
}; static_assert(sizeof(structure_bsp) == 0x288);

static_assert(offsetof(structure_bsp, collision_materials)  == 0x0A4);
static_assert(offsetof(structure_bsp, collision_bsps)       == 0x0B0);
static_assert(offsetof(structure_bsp, map_bounds)           == 0x0C8);
static_assert(offsetof(structure_bsp, leaves)               == 0x0E0);
static_assert(offsetof(structure_bsp, lightmaps)            == 0x104);
static_assert(offsetof(structure_bsp, pathfinding_surfaces) == 0x1E4);

struct structure_bsp::leaf_block_element {
    int32 : 32;
    int16 : 16;
    int16 : 16;

    int16 cluster;
    int16 surface_reference_count;
    int32 first_surface_reference;
}; static_assert(sizeof(structure_bsp::leaf_block_element) == 0x10);

} } // namespace sentinel::tags
