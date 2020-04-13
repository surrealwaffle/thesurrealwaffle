#ifndef SENTINEL__TAGS__EFFECT_HPP
#define SENTINEL__TAGS__EFFECT_HPP

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>

namespace sentinel { namespace tags {

struct effect {
    using location = char[32];
    struct event;

    static constexpr signature tag_signature = make_signature("effe");

    flags_long  flags;
    index_short loop_start_event;
    index_short loop_end_event;

    int32 unknown0[8];

    tag_block<location> locations;
    tag_block<event>    events;
}; static_assert(sizeof(effect) == 0x40);

struct effect::event {
    struct part;
    struct particle;

    int32 unknown0;
    fraction    skip_fraction;
    real_bounds delay_bounds;
    real_bounds duration_bounds;

    int32 PAD0[5];

    tag_block<part>     parts;
    tag_block<particle> particles;
}; static_assert(sizeof(effect::event) == 0x44);

struct effect::event::part {
    enum_short  environment; ///< any, air, water, space
    enum_short  mode;        ///< either, violent, non-violent
    index_short location;    ///< references the effect locations tag block
    flags_short flags;       ///< face down regardless of location

    int32 PAD0[3];

    signature           effect_type;
    tag_reference<void> effect;

    int32 PAD1[6];

    real_bounds velocity; // world units per second
    radians     cone_angle;
    real_bounds angular_velocity; // radians per second
    real_bounds radius_modifier;

    int32 PAD2;

    flags_long scale_modifiers[2];
}; static_assert(sizeof(effect::event::part) == 0x68);

struct effect::event::particle {
    enum_short  environment; ///< any, air, water, space
    enum_short  mode;        ///< either, violent, non-violent
    enum_short  camera_mode; ///< independent, fp, 3p, fp if possible

    int16 PAD0;

    index_short location; ///< references the effect locations tag block

    radians     relative_direction_angles[2]; // yaw, pitch
    real3d      relative_offset;
    direction3d relative_direction; // calculated from relative_direction_angles

    int32 PAD1[10];

    tag_reference<void> particle;

    flags_long flags;
    enum_short distribution_function;

    int16 PAD2;

    short_bounds count;
    real_bounds  distribution_radius;

    int32 PAD3[3];

    real_bounds  velocity;
    radians      velocity_cone_angle;
    real_bounds  angular_velocity;

    int32 PAD4[2];

    real_bounds  radius;

    int32 PAD5[2];

    argbf tint_lower_bound;
    argbf tint_upper_bound;

    int32 PAD6[4];

    flags_long scale_modifiers[2];
}; static_assert(sizeof(effect::event::particle) == 0xE8);

} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__EFFECT_HPP
