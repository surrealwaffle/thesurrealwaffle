
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>
#include <sentinel/fwd/globals_fwd.hpp>

namespace sentinel { namespace tags {

struct collision_bsp;   // sentinel/tags/collision_bsp.hpp
struct collision_model; // sentinel/tags/collision_model.hpp
struct effect;          // sentinel/tags/effect.hpp
struct game_globals;    // sentinel/tags/game_globals.hpp
struct item;            // sentinel/tags/item.hpp
struct object;          // sentinel/tags/object.hpp
struct projectile;      // sentinel/tags/projectile.hpp
struct scenario;        // sentinel/tags/scenario.hpp
struct structure_bsp;   // sentinel/tags/structure_bsp.hpp
struct weapon;          // sentinel/tags/weapon.hpp

} } // namespace sentinel::tags

namespace sentinel {

struct tag_array_element; // sentinel/structures/tag.hpp

template<> struct identity_traits<tag_array_element>
{
    static tags_array_header_type& array()
    { return **sentinel_Globals_globals.tags_array_header; }
};

#define SENTINEL_MAKE_TAG_ID_TRAITS(type, signature_string)                       \
template<> struct is_tag<type> : std::true_type { };                              \
template<> struct identity_traits<type> : identity_traits<tag_array_element>      \
{                                                                                 \
    static constexpr signature type_signature = make_signature(signature_string); \
    using base_type = tag_array_element;                                          \
    static constexpr auto morph =                                                 \
        [] (auto& x) -> type& { return *reinterpret_cast<type*>(x.definition); }; \
}

SENTINEL_MAKE_TAG_ID_TRAITS(tags::collision_model, "coll");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::effect,          "effe");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::game_globals,    "matg");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::item,            "item");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::object,          "obje");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::projectile,      "proj");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::scenario,        "scnr");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::structure_bsp,   "sbsp");
SENTINEL_MAKE_TAG_ID_TRAITS(tags::weapon,          "weap");

#undef SENTINEL_MAKE_TAG_ID_TRAITS

}
