
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

struct game_globals {
    struct interface_bitmaps_block;
    struct player_information_type;

    char UNKNOWN0[248];

    tag_block<void> sounds;
    tag_block<void> camera;
    tag_block<void> player_control;
    tag_block<void> difficulty;
    tag_block<void> grenades;
    tag_block<void> rasterizer_data;
    tag_block<interface_bitmaps_block> interface_bitmaps;
    tag_block<void> weapon_list;
    tag_block<void> cheat_powerups;
    tag_block<void> multiplayer_information;

    tag_block<player_information_type> player_information;

    tag_block<void> first_person_interface;
    tag_block<void> falling_damage;
    tag_block<void> materials;
    tag_block<void> playlist_members;
}; static_assert(sizeof(game_globals) == 0x1AC);

struct game_globals::interface_bitmaps_block {
    tag_reference<void> system_font;
    tag_reference<void> terminal_font;
    tag_reference<void> screen_color_table;
    tag_reference<void> hud_color_table;
    tag_reference<void> editor_color_table;
    tag_reference<void> dialog_color_table;
    tag_reference<void> hud_globals;
    tag_reference<void> motion_sensor_sweep_bitmap;
    tag_reference<void> motion_sensor_sweep_bitmap_mask;
    tag_reference<void> multiplayer_hud_bitmap;
    tag_reference<void> localization;
    tag_reference<void> hud_digits_definition;
    tag_reference<void> motion_sensor_blip_bitmap;
    tag_reference<void> interface_goo_map1;
    tag_reference<void> interface_goo_map2;
    tag_reference<void> interface_goo_map3;
}; static_assert(sizeof(game_globals::interface_bitmaps_block) == 0x100);

struct game_globals::player_information_type {
    tag_reference<void> unit;

    int32 UNKNOWN0[7];

    real walking_speed;
    real double_speed_multiplier;

    struct {
        real forward;
        real backward;
        real sideways;
        real acceleration;
    } run, sneak;

    real  airborne_acceleration;
    real  speed_multiplier; // added to the actual multiplier

    int32 UNKNOWN1[3];

    position3d grenade_origin;

    int32 UNKNOWN2[3];

    struct {
        real movement_penalty;
        real turning_penalty;
        real jumping_penalty;
        real min_time;
        real max_time;
    } stun;

    int32 UNKNOWN3[2];

    struct {
        real min_time;
        real max_time;
        real skip_fraction;
    } first_person_idle;

    int32 UNKNOWN4[4];

    tag_reference<void> coop_respawn_effect;

    int32 UNKNOWN5[11];
}; static_assert(sizeof(game_globals::player_information_type) == 0xF4);

static_assert(offsetof(game_globals, sounds) == 0x0F8);
static_assert(offsetof(game_globals, camera) == 0x104);
static_assert(offsetof(game_globals, player_control) == 0x110);
static_assert(offsetof(game_globals, difficulty) == 0x11C);
static_assert(offsetof(game_globals, grenades) == 0x128);
static_assert(offsetof(game_globals, rasterizer_data) == 0x134);
static_assert(offsetof(game_globals, interface_bitmaps) == 0x140);
static_assert(offsetof(game_globals, weapon_list) == 0x14C);
static_assert(offsetof(game_globals, cheat_powerups) == 0x158);
static_assert(offsetof(game_globals, multiplayer_information) == 0x164);
static_assert(offsetof(game_globals, player_information) == 0x170);
static_assert(offsetof(game_globals, first_person_interface) == 0x17C);
static_assert(offsetof(game_globals, falling_damage) == 0x188);
static_assert(offsetof(game_globals, materials) == 0x194);
static_assert(offsetof(game_globals, playlist_members) == 0x1A0);

static_assert(offsetof(game_globals::interface_bitmaps_block, system_font)                     == 0x00);
static_assert(offsetof(game_globals::interface_bitmaps_block, terminal_font)                   == 0x10);
static_assert(offsetof(game_globals::interface_bitmaps_block, screen_color_table)              == 0x20);
static_assert(offsetof(game_globals::interface_bitmaps_block, hud_color_table)                 == 0x30);
static_assert(offsetof(game_globals::interface_bitmaps_block, editor_color_table)              == 0x40);
static_assert(offsetof(game_globals::interface_bitmaps_block, dialog_color_table)              == 0x50);
static_assert(offsetof(game_globals::interface_bitmaps_block, hud_globals)                     == 0x60);
static_assert(offsetof(game_globals::interface_bitmaps_block, motion_sensor_sweep_bitmap)      == 0x70);
static_assert(offsetof(game_globals::interface_bitmaps_block, motion_sensor_sweep_bitmap_mask) == 0x80);
static_assert(offsetof(game_globals::interface_bitmaps_block, multiplayer_hud_bitmap)          == 0x90);
static_assert(offsetof(game_globals::interface_bitmaps_block, localization)                    == 0xA0);
static_assert(offsetof(game_globals::interface_bitmaps_block, hud_digits_definition)           == 0xB0);
static_assert(offsetof(game_globals::interface_bitmaps_block, motion_sensor_blip_bitmap)       == 0xC0);
static_assert(offsetof(game_globals::interface_bitmaps_block, interface_goo_map1)              == 0xD0);
static_assert(offsetof(game_globals::interface_bitmaps_block, interface_goo_map2)              == 0xE0);
static_assert(offsetof(game_globals::interface_bitmaps_block, interface_goo_map3)              == 0xF0);

static_assert(offsetof(game_globals::player_information_type, airborne_acceleration) == 0x54);
static_assert(offsetof(game_globals::player_information_type, coop_respawn_effect) == 0xB8);

} } // namespace sentinel::tags
