
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef> // offsetof

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>
#include <sentinel/structures/tag.hpp>

namespace sentinel {

/* sizes of objects and their composition:
   numbers account for the base datum size (i.e. biped-specific datum is 0x550 - 0x44C bytes, discounting alignment padding)
   this information can be gleaned from the object prototypes
    biped 0x550
        object  0x1F4
        unit    0x4CC
        biped   0x550
    vehicle 0x5C0
        object  0x1F4
        unit    0x4CC
        vehicle 0x5C0
    weapon 0x340
        object  0x1F4
        item    0x22C
        weapon  0x340
    equipment 0x294
        object      0x1F4
        item        0x22C
        equipment   0x294
    garbage 0x244
        object  0x1F4
        item    0x22C
        garbage 0x244
    projectile 0x2B0
        object      0x1F4
        projectile  0x2B0
    scenery 0x1F8
        object  0x1F4
        scenery 0x1F8
    machine 0x228
        object  0x1F4
        device  0x214
        machine 0x228
    control 0x21C
        object  0x1F4
        device  0x214
        control 0x21C
    light_fixture 0x22c
        object          0x1F4
        device          0x214
        light_fixture   0x22C
    placeholder 0x1FC
        object      0x1F4
        placeholder 0x1FC
    sound_scenery 0x1F8
        object          0x1F4
        sound_scenery   0x1F8
*/

struct object;

struct unit;
struct vehicle;
struct biped;

struct item;
struct weapon;
struct equipment;
struct garbage;

struct projectile;
struct scenery;
struct placeholder;
struct sound_scenery;

struct device;
struct machine;
struct control;
struct light_fixture;

struct object_table_datum {
    identity_salt salt;
    flags_byte    flags; ///< (0): ??, (1): ??, (2): ??, (3): pending_deletion
    enum_byte     type;
    index_short   cluster;
    int16         object_size; ///< Size of the entire allocation
    object*       object;

    /*
    union {
        object  *object;

        unit    *unit;
        vehicle *vehicle; // type = 1
        biped   *biped;   // type = 0

        item      *item;
        weapon    *weapon; // type = 2
        equipment *equipment; // type = 3
        garbage   *garbage; // type = 4

        projectile    *projectile; // type = 5
        scenery       *scenery; // type = 6
        placeholder   *placeholder; // type = 10
        sound_scenery *sound_scenery; // type = 11

        device        *device;
        machine       *machine; // type = 7
        control       *control; // type = 8
        light_fixture *light_fixture; // type = 9
    };
    */
}; static_assert(sizeof(object_table_datum) == 0x0C);

template<class T, std::ptrdiff_t FieldOffset>
struct object_datum_dynamic_block {
    h_short size;    // allocation size, not sure if signed or unsigned
    h_short  offset; // yes, its signed

    T*       data()       { return reinterpret_cast<T*>(reinterpret_cast<char*>(this) + (offset - FieldOffset)); }
    const T* data() const { return reinterpret_cast<const T*>(reinterpret_cast<const char*>(this) + (offset - FieldOffset)); }

    T&       operator[](std::size_t pos)       { return data()[pos]; }
    const T& operator[](std::size_t pos) const { return data()[pos]; }
};

struct object_datum {
    identity<tag_array_element> tag;

    int32 unknown00[22];

    position3d position; ///< The position, relative to the #parent.
    real3d     velocity; ///< The velocity, relative to the #parent.

    // need to test if these are relative to the parent or not
    direction3d forward;
    direction3d up;

    real3d angular_velocity;

    int32 unknown01[2];

    position3d bound_center;
    real       bound_radius;
    real       scale;

    enum_short type; ///< 0=biped,1=vehicle,2=weapon,3=equipment,4=garbage,5=projectile,6=scenery,7=machine,8=control,9=light_fixture,10=placeholder,11=sound_scenery

    int32 unknown02[19];

    int16 unknown03;

    flags_short damage_flags;

    int32 unknown04;

    identity</*TODO*/> object_cluster_reference; // collideable_object_cluster_reference
    int32 unknown05;
    identity<object> next_sibling; ///< The next sibling in the object hierarchy.
    identity<object> first_child;  ///< The first child in the object hierarchy.
    identity<object> parent;       ///< The parent object in the object hierarchy.

    int32 unknown06[52];

    object_datum_dynamic_block<affine_matrix3d, 0x1F0> node_transforms;
}; static_assert(sizeof(object_datum) == 0x1F4);

struct object {
    object_datum object;
};

struct object_marker_result_type {
    index_short marker_node; ///< The index of the node that the marker is a direct child of.

    affine_matrix3d marker_transform; ///< Taken from the tag, translation is the marker
                                      ///< position relative to the node
    affine_matrix3d world_transform; ///< Computed by NodeToWorld * marker_transform,
                                     ///< translation is the marker position in world space.
}; static_assert(sizeof(object_marker_result_type) == 0x6C);

struct object_prototype {
    using update_proc_type SENTINEL_ATTRIBUTES(cdecl) = bool(*)(identity<object>);

    char const* name; ///< The object type name.
    int32 type; ///< The type signature.
    int16 size; ///< The size of the allocation, in bytes.

    int16 unknown00[3];
    int32 unknown01;
    void* proc_unknown1[8];
    update_proc_type proc_update;
    void* proc_unknown2[18];

    object_prototype* bases[16]; // fills out from index 0, with null sentinel indicating the end, usually self-references

    int32 unknown02[2];
}; static_assert(sizeof(object_prototype) == 0xC8);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(object_table_datum, salt) == 0x00);
static_assert(offsetof(object_table_datum, flags) == 0x02);
static_assert(offsetof(object_table_datum, type) == 0x03);
static_assert(offsetof(object_table_datum, cluster) == 0x04);
static_assert(offsetof(object_table_datum, object_size) == 0x06);
static_assert(offsetof(object_table_datum, object) == 0x08);

static_assert(offsetof(object_datum, tag) == 0x000);
static_assert(offsetof(object_datum, position) == 0x05C);
static_assert(offsetof(object_datum, velocity) == 0x068);
static_assert(offsetof(object_datum, forward) == 0x074);
static_assert(offsetof(object_datum, up) == 0x080);
static_assert(offsetof(object_datum, angular_velocity) == 0x08C);
static_assert(offsetof(object_datum, bound_center) == 0x0A0);
static_assert(offsetof(object_datum, type) == 0x0B4);
static_assert(offsetof(object_datum, damage_flags) == 0x106);
static_assert(offsetof(object_datum, object_cluster_reference) == 0x10C);
static_assert(offsetof(object_datum, next_sibling) == 0x114);
static_assert(offsetof(object_datum, first_child) == 0x118);
static_assert(offsetof(object_datum, parent) == 0x11C);

static_assert(offsetof(object_datum, node_transforms) == 0x1F0);

static_assert(offsetof(object_marker_result_type, marker_node) == 0x00);
static_assert(offsetof(object_marker_result_type, marker_transform) == 0x04);
static_assert(offsetof(object_marker_result_type, world_transform) == 0x38);

static_assert(offsetof(object_prototype, name) == 0x00);
static_assert(offsetof(object_prototype, type) == 0x04);
static_assert(offsetof(object_prototype, size) == 0x08);
static_assert(offsetof(object_prototype, proc_unknown1) == 0x14);
static_assert(offsetof(object_prototype, proc_update) == 0x34);
static_assert(offsetof(object_prototype, proc_unknown2) == 0x38);
static_assert(offsetof(object_prototype, bases) == 0x80);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

} // namespace sentinel
