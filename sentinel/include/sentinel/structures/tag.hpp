
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include <sentinel/types.hpp>
#include <sentinel/fwd/globals_fwd.hpp>
#include <sentinel/fwd/tags_fwd.hpp>

#include <sentinel/tags/scenario.hpp>

namespace sentinel { namespace tags {

struct scenario;
struct game_globals;
struct weapon;
struct projectile;
struct structure_bsp;
struct collision_model;
struct effect;

} } // namespace sentinel::tags

namespace sentinel {

struct tag_array_element {
    signature   types[3]; ///< The tag types (or `-1` for no type), from most to least specific.
    identity<tag_array_element> identity; ///< The tag identity.
    h_ccstr     name;     ///< The tag's path.
    void*       definition;

    /*
    union {
        void*                 definition;
        tags::collision_model *collision_model;
        tags::effect          *effect;
        tags::game_globals    *game_globals;
        tags::item            *item;
        tags::object          *object;
        tags::projectile      *projectile;
        tags::weapon          *weapon;
        tags::scenario        *scenario;
        tags::structure_bsp   *structure_bsp;
    };
    */

    int32 unknown00; // 0x18 - not sure type or meaning
    int32 unknown01; // 0x1C - not sure type or meaning

    bool has_type(const sentinel::signature& type) const
    { return types[0] == type || types[1] == type || types[2] == type; }
}; static_assert(sizeof(tag_array_element) == 0x20);

struct tags_array_header_type {
    using pointer         = tag_array_element*;
    using const_pointer   = tag_array_element const*;
    using reference       = tag_array_element&;
    using const_reference = tag_array_element const&;
    using iterator        = pointer;
    using const_iterator  = const_pointer;

    pointer    tags;     ///< The array of \ref sentinel::tag_reference "tag references".
    identity<tags::scenario> scenario; ///< The identity of the scenario tag in \a tags.
    int32      unknown00; // 0x08 - not sure type or meaning
    int32      count;    ///< The number of tags in \a tags.
    int32      unknown01[5]; // 0x10 - not sure type or meaning, last one is model data size
    signature  TAGS;     ///< `"tags"_hsig`

    /** Returns a reference to the tag reference at \a index, which may be a \ref sentinel::identity or integral value. */
    reference operator[](const int index) { return tags[index]; }

    /** Returns a constant reference to the tag reference at \a index, which may be a \ref sentinel::identity or integral value. */
    const_reference operator[](const int index) const { return tags[index]; }

    iterator begin() noexcept { return tags; }
    const_iterator begin() const noexcept { return tags; }
    const_iterator cbegin() const noexcept { return tags; }

    iterator end() noexcept { return tags + count; }
    const_iterator end() const noexcept { return tags + count; }
    const_iterator cend() const noexcept { return tags + count; }
}; static_assert(sizeof(tags_array_header_type) == 0x28);

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(tags_array_header_type, tags)     == 0x00);
static_assert(offsetof(tags_array_header_type, scenario) == 0x04);
static_assert(offsetof(tags_array_header_type, count)    == 0x0C);

static_assert(offsetof(tag_array_element, types)      == 0x00);
static_assert(offsetof(tag_array_element, identity)   == 0x0C);
static_assert(offsetof(tag_array_element, name)       == 0x10);
static_assert(offsetof(tag_array_element, definition) == 0x14);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

} // namespace sentinel

