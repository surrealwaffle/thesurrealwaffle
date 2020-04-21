
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#ifndef SENTINEL__TAGS__ITEM_HPP
#define SENTINEL__TAGS__ITEM_HPP

#include <sentinel/types.hpp>
#include <sentinel/tags/generic.hpp>
#include <sentinel/tags/object.hpp>

namespace sentinel { namespace tags {
    struct item_definition {
        int32 flags;

        int16 message_index;
        int16 sort_order;
        real  scale;
        int16 hud_message_value_scale;

        int32 UNKNOWN0[4];

        int16 function_exports[4];

        int32 UNKNOWN1[41];

        tag_reference<void> material_effects;
        tag_reference<void> collision_sound;

        int32 UNKNOWN2[30];

        struct {
            real min_delay;
            real max_delay;

            tag_reference<void> detonating_effect;
            tag_reference<void> detonation_effect;
        } detonation;
    }; static_assert(sizeof(item_definition) == 0x18C);

    struct item : object {
        item_definition item;
    }; static_assert(sizeof(item) == 0x308);
} } // namespace sentinel::tags

#endif // SENTINEL__TAGS__ITEM_HPP
