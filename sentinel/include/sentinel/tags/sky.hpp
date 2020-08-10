
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

struct sky {
    struct radiosity_spec {
        rgbf color;
        real power;
    }; static_assert(sizeof(radiosity_spec) == 0x10);

    struct fog_spec {
        rgbf        color;
        uint32      unknown0[2];
        fraction    maximum_density;
        real_bounds range; // [start_distance, opaque_distance
    }; static_assert(sizeof(fog_spec) == 0x20);

    struct light;

    tag_reference<void> model;
    tag_reference<void> animation_graph;
    uint32 unknown0[6];

    radiosity_spec indoor_ambient_radiosity;
    radiosity_spec outdoor_ambient_radiosity;

    fog_spec outdoor_fog;
    fog_spec indoor_fog;

    tag_reference<void> fog_screen;

    uint32 unknown1[7]; // tag_block<shader_function> and tag_block<animations> in here

    tag_block<light> lights;
}; static_assert(sizeof(sky) == 0xD0);

struct sky::light {
    tag_reference<void> lens;
    uint32 unknown0[15]; // lens_flare_marker_name somewhere in here, possibly this entire member

    flags_long flags;
    rgbf color;
    real power;
    real test_distance; // length of ray used to test for shadows

    uint32 unknown1;

    radians yaw;
    radians pitch;
    radians angular_diameter;
}; static_assert(sizeof(sky::light) == 0x74);

static_assert(offsetof(sky, model) == 0x00);
static_assert(offsetof(sky, animation_graph) == 0x10);
static_assert(offsetof(sky, indoor_ambient_radiosity) == 0x38);
static_assert(offsetof(sky, outdoor_ambient_radiosity) == 0x48);
static_assert(offsetof(sky, outdoor_fog) == 0x58);
static_assert(offsetof(sky, indoor_fog) == 0x78);
static_assert(offsetof(sky, fog_screen) == 0x98);
static_assert(offsetof(sky, lights) == 0xC4);

static_assert(offsetof(sky::light, lens) == 0x00);
static_assert(offsetof(sky::light, flags) == 0x4C);
static_assert(offsetof(sky::light, color) == 0x50);
static_assert(offsetof(sky::light, power) == 0x5C);
static_assert(offsetof(sky::light, test_distance) == 0x60);
static_assert(offsetof(sky::light, yaw) == 0x68);
static_assert(offsetof(sky::light, pitch) == 0x6C);
static_assert(offsetof(sky::light, angular_diameter) == 0x70);

} } // namespace sentinel::tags
