
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__TAGS__COLLISION_MODEL_HPP
#define SENTINEL__TAGS__COLLISION_MODEL_HPP

#include <cstddef> // offsetof

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

struct collision_model {
    static constexpr signature tag_signature = make_signature("coll");

    flags_long  flags;
    index_short indirect_damage_material;

    struct pathfinding_sphere;

    struct {
        real maximum_vitality;
        real system_shock;

        int32 PAD0[13];

        fraction friendly_damage_resistance;

        int32 PAD1[10];

        tag_reference<void> localized_damage_effect;
        fraction            area_damage_effect_threshold;
        tag_reference<void> area_damage_effect;

        real                damaged_threshold;
        tag_reference<void> damaged_effect;
        tag_reference<void> depleted_effect;
        real                destroyed_threshold;
        tag_reference<void> destroyed_effect;
    } body;

    static_assert(sizeof(body) == 0xC4);

    struct {
        real       maximum_vitality;

        int16 PAD0;

        enum_short material_type;

        int32 PAD1[6];

        enum_short failure_function;
        fraction   failure_threshold;
        fraction   failing_shield_leak;

        int32 PAD2[4];

        real       minimum_stun_damage;
        real       stun_time;
        real       recharge_time;

        int32 PAD3[28];

        real       damaged_threshold;
        tag_reference<void> damaged_effect;
        tag_reference<void> depleted_effect;
        tag_reference<void> recharging_effect;
    } shield;

    static_assert(sizeof(shield) == 0xEC);

    int32 PAD0[31];

    tag_block<void> materials;
    tag_block<void> regions;
    tag_block<void> modifiers;

    int32 PAD1[4];

    struct {
        real_bounds                   box[3]; // bounds: x, y, z in that order
        tag_block<pathfinding_sphere> spheres;
    } pathfinding;

    tag_block<void> nodes;
}; static_assert(sizeof(collision_model) == 0x298);

struct collision_model::pathfinding_sphere {
    index_short node;
    int32       unknown00[3];
    position3d  center;
    real        radius;
}; static_assert(sizeof(collision_model::pathfinding_sphere) == 0x20);

static_assert(offsetof(collision_model, materials) == 0x234);
static_assert(offsetof(collision_model, pathfinding) == 0x268);

} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__COLLISION_MODEL_HPP
