#pragma once

#include <sentinel/config.hpp>
#include <sentinel/types.hpp>

namespace sentinel {

struct effect;                // sentinel/structures/effect.hpp
struct object_table_datum;    // sentinel/structures/object.hpp
struct player;                // sentinel/structures/player.hpp
struct terminal_output_entry; // sentinel/structures/console.hpp
struct script_node_type;      // sentinel/structures/script.hpp
struct script_thread_type;    // sentinel/structures/script.hpp

struct object;  // sentinel/structures/object.hpp
struct unit;    // sentinel/structures/unit.hpp
struct vehicle; // <undefined>
struct biped;   // sentinel/structures/biped.hpp

struct item;      // sentinel/structures/item.hpp
struct weapon;    // sentinel/structures/weapon.hpp
struct equipment; // <undefined>
struct garbage;   // <undefined>

struct projectile;    // <undefined>
struct scenery;       // <undefined>
struct placeholder;   // <undefined>
struct sound_scenery; // <undefined>

struct device;        // <undefined>
struct machine;       // <undefined>
struct control;       // <undefined>
struct light_fixture; // <undefined>

template<class DatumType>
struct table_type;

struct table_aggregate_type {
    table_type<effect>*                effect_table;
    table_type<object_table_datum>*    object_table;
    table_type<player>*                player_table;
    table_type<terminal_output_entry>* terminal_output_table;
    table_type<script_node_type>*      script_node_table;
    table_type<script_thread_type>*    script_thread_table;
};

} // namespace sentinel

extern "C" {

extern
SENTINEL_API
sentinel::table_aggregate_type sentinel_Globals_tables;

} // extern "C""

namespace sentinel {

#define SENTINEL_MAKE_TABLE_ID_TRAITS(type, table_name)                  \
template<> struct identity_traits<type>                                  \
{                                                                        \
    static auto& array() { return *sentinel_Globals_tables.table_name; } \
}

SENTINEL_MAKE_TABLE_ID_TRAITS(effect, effect_table);
SENTINEL_MAKE_TABLE_ID_TRAITS(object_table_datum, object_table);
SENTINEL_MAKE_TABLE_ID_TRAITS(player, player_table);
SENTINEL_MAKE_TABLE_ID_TRAITS(terminal_output_entry, terminal_output_table);
SENTINEL_MAKE_TABLE_ID_TRAITS(script_node_type, script_node_table);
SENTINEL_MAKE_TABLE_ID_TRAITS(script_thread_type, script_thread_table);

#undef SENTINEL_MAKE_TABLE_ID_TRAITS

#define SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(type)                          \
template<> struct identity_traits<type>                                     \
{                                                                           \
    using base_type = object_table_datum;                                   \
    static auto& array() { return *sentinel_Globals_tables.object_table; }  \
    static constexpr auto morph = [] (auto& x) -> type&                     \
        { return *reinterpret_cast<type*>(x.object);  };                    \
}

SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(object);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(unit);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(vehicle);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(biped);

SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(item);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(weapon);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(equipment);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(garbage);

SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(projectile);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(scenery);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(placeholder);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(sound_scenery);

SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(device);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(machine);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(control);
SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS(light_fixture);

#undef SENTINEL_MAKE_OBJECT_TABLE_ID_TRAITS

} // namespace sentinel
