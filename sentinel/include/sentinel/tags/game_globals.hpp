#ifndef SENTINEL__TAGS__GLOBALS_HPP
#define SENTINEL__TAGS__GLOBALS_HPP

#include <cstddef>

#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

    struct game_globals {
        struct player_information_type;

        char UNKNOWN0[248];

        tag_block<void> sounds;
        tag_block<void> camera;
        tag_block<void> player_control;
        tag_block<void> difficulty;
        tag_block<void> grenades;
        tag_block<void> rasterizer_data;
        tag_block<void> interface_bitmaps;
        tag_block<void> weapon_list;
        tag_block<void> cheat_powerups;
        tag_block<void> multiplayer_information;

        tag_block<player_information_type> player_information;

        tag_block<void> first_person_interface;
        tag_block<void> falling_damage;
        tag_block<void> materials;
        tag_block<void> playlist_members;
    }; static_assert(sizeof(game_globals) == 0x1AC);

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

    static_assert(offsetof(game_globals::player_information_type, airborne_acceleration) == 0x54);
    static_assert(offsetof(game_globals::player_information_type, coop_respawn_effect) == 0xB8);
} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__GLOBALS_HPP
