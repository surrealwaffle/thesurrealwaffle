
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/item.hpp>
#include <sentinel/tags/projectile.hpp>

namespace sentinel { namespace tags {

struct weapon_definition {
    struct trigger;

    int32 flags;

    char label[32];
    int16 secondary_trigger_mode;
    int16 maximum_alternate_shots_loaded;

    int16 function_exports[4];

    real ready_time;
    tag_reference<void> ready_effect;

    struct {
        real recovery_threshold;
        real overheated_threshold;
        real detonation_threshold;
        real detonation_fraction;
        real loss_per_second;
        real illumination;

        int32 : 32;
        int32 : 32;
        int32 : 32;
        int32 : 32;

        tag_reference<void> overheated;
        tag_reference<void> detonation;
    } heat;

    tag_reference<void> player_melee_damage;
    tag_reference<void> player_melee_response;

    int32 : 32;
    int32 : 32;

    tag_reference<void> actor_firing_parameters;

    struct {
        real near_range;
        real far_range;
        real intersection_range;
    } reticle;

    struct {
        int16 : 16;

        int16 levels;
        real  range_lower;
        real  range_upper;
    } zoom;

    struct {
        real autoaim_angle;
        real autoaim_range;
        real magnetism_angle;
        real magnetism_range;
        real deviation_angle;
    } aim_assist;

    int32 : 32;

    struct {
        int16 movement_penalized;
        real forward_movement_penalty;
        real sideways_movement_penalty;
    } movement;

    int32 : 32;

    struct {
        real minimum_target_range;
        real looking_time_modifier;
    } ai_targeting_parameters;

    int32 : 32;

    struct {
        real power_on_time;
        real power_off_time;
        tag_reference<void> power_on_effect;
        tag_reference<void> power_off_effect;
    } light;

    struct {
        real heat_recovery_penalty;
        real rate_of_fire_penalty;
        real misfire_start;
        real misfire_chance;
    } age;

    int32 : 32;
    int32 : 32;
    int32 : 32;

    struct {
        tag_reference<void> first_person_model;
        tag_reference<void> first_person_animations;

        int32 : 32;

        tag_reference<void> hud_interface;
        tag_reference<void> pickup_sound;
        tag_reference<void> zoom_in_sound;
        tag_reference<void> zoom_out_sound;

        int32 : 32;
        int32 : 32;
        int32 : 32;

        real active_camo_ding;
        real active_camo_regrowth_rate;
    } weapon_interface;

    int32 : 32;
    int32 : 32;
    int32 : 32;

    int16 : 16;
    int16 weapon_type;

    tag_block<void> predicted_resources;
    tag_block<void> magazines;
    tag_block<trigger> triggers;
}; static_assert(sizeof(weapon_definition) == 0x200);

struct weapon : item {
    weapon_definition weapon;
}; static_assert(sizeof(weapon) == 0x508);

struct weapon_definition::trigger {
    int32 flags;

    struct {
        real_bounds rounds_per_second;
        real acceleration_time;
        real deceleration_time;
        real blurred_rate_of_fire;

        int32 : 32;
        int32 : 32;

        int16 magazine;
        int16 rounds_per_shot;
        int16 minimum_rounds_loaded;
        int16 rounds_between_tracers;

        int32 : 32;
        int16 : 16;

        int16 firing_noise;
    } firing;

    struct {
        real_bounds bounds;
        real acceleration_time;
        real deceleration_time;
    } error;

    int32 : 32;
    int32 : 32;

    struct {
        real charging_time;
        real charged_time;
        int16 overcharged_action;
        real charged_illumination;
        real spew_time;
        tag_reference<void> charging_effect;
    } charging;

    struct {
        int16 distribution_function;
        int16 projectiles_per_shot;
        real distribution_angle;

        int32 : 32;

        real minimum_error;
        real minimum_error_angle;
        real maximum_error_angle;
        position3d first_person_offset;

        int32 : 32;

        tag_reference<projectile> projectile;
    } projectile;

    real ejection_port_recovery_time;
    real illumination_recovery_time;

    int32 : 32;
    int32 : 32;
    int32 : 32;

    real heat_generated_per_round;
    real age_generated_per_round;

    int32 : 32;

    real overload_time;

    int32 UNKNOWN0[16];

    tag_block<void> firing_effects;
}; static_assert(sizeof(weapon_definition::trigger) == 0x114);

} } // namespace sentinel::tags
