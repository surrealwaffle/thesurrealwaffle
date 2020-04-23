
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/globals.hpp>
#include <sentutil/utility.hpp>

namespace sentutil { namespace globals {

using utility::heavy_pointer_wrapper;

inline sentinel::globals_aggregate_type& sentinel_globals = sentinel_Globals_globals;
inline sentinel::table_aggregate_type&   sentinel_tables  = sentinel_Globals_tables;

inline heavy_pointer_wrapper game_time_globals([] { return *sentinel_globals.game_time_globals; });
inline heavy_pointer_wrapper local_player_globals([] { return *sentinel_globals.local_player_globals; });
inline heavy_pointer_wrapper tags_array([]  { return *sentinel_globals.tags_array_header; });
inline heavy_pointer_wrapper allocator_globals([] { return sentinel_globals.allocator_globals; });
inline heavy_pointer_wrapper camera_globals([]  { return sentinel_globals.camera_globals; });
// inline heavy_pointer_wrapper chat_globals([] { return sentinel_globals.chat_globals; });
inline heavy_pointer_wrapper console_globals([] { return sentinel_globals.console_globals; });
inline heavy_pointer_wrapper machine_globals([] { return sentinel_globals.machine_globals; });
inline heavy_pointer_wrapper map_globals([] { return sentinel_globals.map_globals; });
inline heavy_pointer_wrapper runtime_sound_globals([] { return sentinel_globals.runtime_sound_globals; });
// inline heavy_pointer_wrapper terminal([] { return sentinel_globals.terminal; });
inline heavy_pointer_wrapper map_file_header([] { return sentinel_globals.map_file_header; });

inline heavy_pointer_wrapper effects([] { return sentinel_tables.effect_table; });
inline heavy_pointer_wrapper objects([] { return sentinel_tables.object_table; });
inline heavy_pointer_wrapper players([] { return sentinel_tables.player_table; });
// inline heavy_pointer_wrapper terminal_output([] { return sentinel_tables.terminal_output_table; });
inline heavy_pointer_wrapper script_nodes([] { return sentinel_tables.script_node_table; });
inline heavy_pointer_wrapper script_threads([] { return sentinel_tables.script_thread_table; });

inline sentinel::GameEdition& game_edition = sentinel_Globals_Edition;

/** \brief Checks the command line parameters for a string.
 *
 * \param[in] target The switch to search for. Must be prefixed by `-`.
 *
 * \return `true` if \a target is among the command-line parameters, or
 *         `false` if \a target could not be found.
 */
bool get_command_line(const char* target);

/** \brief Checks the command line parameters for a string.
 *
 * Behavior of this function concerning \a target_value does not affect the returned
 * value; the returned value is dependent strictly on \a target itself.
 *
 * If the argument following \a target exists and is not prefixed by `-`,
 * \a target_value is assigned that argument.
 * Otherwise, \a target_value is assigned `nullptr`.
 *
 * \param[in]  target       The switch to search for. Must be prefixed by `-`.
 * \param[out] target_value Receives the parameter value passed for \a target.
 *
 * \return `true` if \a target is among the command-line parameters, or
 *         `false` if \a target could not be found.
 */
bool get_command_line(const char* target, const char*& target_value);

} } // namespace globals
