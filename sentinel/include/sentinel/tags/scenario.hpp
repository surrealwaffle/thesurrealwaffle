
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__TAGS__SCENARIO_HPP
#define SENTINEL__TAGS__SCENARIO_HPP

#include <cstddef>

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/structure_bsp.hpp>
#include <sentinel/fwd/table_fwd.hpp>

namespace sentinel { namespace tags {

struct scenario {
    struct scenery_block_element;
    struct netgame_flags_block_element;
    struct netgame_equipment_block_element;
    struct starting_equipment_block_element;
    struct structure_bsp_block_element;

    tag_reference<void> dont_use_; ///< SBSP
    tag_reference<void> wont_use_; ///< SBSP
    tag_reference<void> cant_use_; ///< SKY

    tag_block<void> skies;
    int16           type;
    int16           flags;
    tag_block<void> child_scenarios;
    real            local_north;

    int32 unknown0[112]; // Quite a bit of stuff in here but not much is useful for MP

    tag_block<scenery_block_element> scenery;
    tag_block<tag_reference<void>>   scenery_palette;

    tag_block<void> bipeds;
    tag_block<tag_reference<void>> biped_palette;

    tag_block<void> vehicles;
    tag_block<tag_reference<void>> vehicle_palette;

    tag_block<void> equipment;
    tag_block<tag_reference<void>> equipment_palette;

    tag_block<void> weapons;
    tag_block<tag_reference<void>> weapon_palette;

    tag_block<void> device_groups;

    tag_block<void> machines;
    tag_block<tag_reference<void>> machine_palette;

    tag_block<void> controls;
    tag_block<tag_reference<void>> control_palette;

    tag_block<void> light_fixtures;
    tag_block<tag_reference<void>> light_fixtures_palette;

    tag_block<void> sound_scenery;
    tag_block<tag_reference<void>> sound_scenery_palette;

    int32 unknown1[24]; // player profile is in here somewhere

    tag_block<void> player_starting_locations;

    tag_block<void> trigger_volumes;

    tag_block<void> recorded_animations;

    tag_block<netgame_flags_block_element> netgame_flags;
    tag_block<netgame_equipment_block_element> netgame_equipment;
    tag_block<starting_equipment_block_element> starting_equipment;

    tag_block<void> bsp_switch_trigger_volumes;

    tag_block<void> decals;
    tag_block<tag_reference<void>> decal_palette;

    int32 unknown2[48];

    table_type<script_node_type> *script_node;

    int32 unknown3[72]; // mostly scripting, AI, globals, cutscenes

    tag_block<structure_bsp_block_element> structure_bsps;
}; static_assert(sizeof(scenario) == 0x5B0);

static_assert(offsetof(scenario, local_north) == 0x4C);
static_assert(offsetof(scenario, scenery) == 0x210);
static_assert(offsetof(scenario, sound_scenery) == 0x2DC);
static_assert(offsetof(scenario, player_starting_locations) == 0x354);
static_assert(offsetof(scenario, starting_equipment) == 0x390);
static_assert(offsetof(scenario, bsp_switch_trigger_volumes) == 0x39C);
static_assert(offsetof(scenario, decals) == 0x3A8);
static_assert(offsetof(scenario, decal_palette) == 0x3B4);

static_assert(offsetof(scenario, structure_bsps) == 0x5A4);

struct scenario::scenery_block_element {
    int16 type; ///< The palette index of the scenery object..
    int16 name; ///< The index for the name of the object, or `-1` if it has no name.
    int16 not_placed_flags;
    int16 desired_permutation;
    position3d position; ///< Position of the scenery object.
    real3d rotation; ///< Rotation of the scenery object, as Euler angles.

    int32 PAD[10]; // no idea
}; static_assert(sizeof(scenario::scenery_block_element) == 0x48);

struct scenario::netgame_flags_block_element {
    enum TYPE : int16 {
        CTF_FLAG,
        CTF_VEHICLE,
        ODDBALL_SPAWN,
        RACE_TRACK,
        RACE_VEHICLE,
        VEGAS_BANK,
        TELEPORT_FROM,
        TELEPORT_TO,
        HILL_FLAG
    };

    position3d position;
    real  facing;
    TYPE  type;
    int16 team_index;
    tag_reference<void> weapon_group; ///< Item collection that spawns equipment

    int32 PAD[28]; // no idea
}; static_assert(sizeof(scenario::netgame_flags_block_element) == 0x94);
static_assert(scenario::netgame_flags_block_element::TELEPORT_FROM == 6);

struct scenario::netgame_equipment_block_element {
    int32 flags;
    int16 type[4];
    int16 team_index;
    int16 spawn_time;
    identity<sentinel::object> object; // physical entity, not tag

    int32 PAD0[11];

    real3d position;
    real  facing;
    tag_reference<void> item_collection; ///< Item collection that spawns equipment

    int32 PAD1[12];
}; static_assert(sizeof(scenario::netgame_equipment_block_element) == 0x90);

struct scenario::starting_equipment_block_element {
    int32 flags;
    int16 type[4];

    int32 PAD0[12];

    tag_reference<void> item_collection[6];

    int32 PAD1[12];
}; static_assert(sizeof(scenario::starting_equipment_block_element) == 0xCC);

static_assert(offsetof(scenario::starting_equipment_block_element, item_collection) == 0x3C);

struct scenario::structure_bsp_block_element {
    int32 : 32; // file offset
    int32 : 32; // probably size related
    structure_bsp_header* structure_bsp_header;
    int32 : 32; // ?? zero?
    tag_reference<structure_bsp> structure_bsp;
}; static_assert(sizeof(scenario::structure_bsp_block_element) == 0x20);
} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__SCENARIO_HPP
