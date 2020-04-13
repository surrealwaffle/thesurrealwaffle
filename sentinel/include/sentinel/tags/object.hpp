#ifndef SENTINEL__TAGS__OBJECT_HPP
#define SENTINEL__TAGS__OBJECT_HPP

#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/collision_model.hpp>
#include <sentinel/types.hpp>

namespace sentinel { namespace tags {
    struct object_definition {
        int16 type;
        int16 flags;
        real bounding_radius;
        position3d bounding_offset;
        position3d origin_offset;
        real acceleration_scale;

        int32 unknown0;;

        tag_reference<void> model;
        tag_reference<void> animation_graph;

        int32 unknown1[10];

        tag_reference<collision_model> collision_model;
        tag_reference<void> physics;
        tag_reference<void> modifier_shader;
        tag_reference<void> creation_effect;

        int32 unknown2[21];

        real render_bounding_radius;

        index_short function_exports[4];

        int32 unknown3[11];

        index_short hud_text_message_index;
        index_short forced_shader_permutation_index;

        tag_block<void> attachments;
        tag_block<void> widgets;
        tag_block<void> functions;
        tag_block<void> change_colors;
        tag_block<void> predicted_resource;
    }; static_assert(sizeof(object_definition) == 0x17C);

    static_assert(offsetof(object_definition, type) == 0x000);
    static_assert(offsetof(object_definition, flags) == 0x002);
    static_assert(offsetof(object_definition, bounding_radius) == 0x004);
    static_assert(offsetof(object_definition, bounding_offset) == 0x008);
    static_assert(offsetof(object_definition, origin_offset) == 0x014);
    static_assert(offsetof(object_definition, acceleration_scale) == 0x020);

    static_assert(offsetof(object_definition, functions) == 0x158);

    struct object {
        object_definition object;
    };

    struct scenery : object { };
} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__OBJECT_HPP
