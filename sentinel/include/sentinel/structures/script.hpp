
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/types.hpp>

namespace sentinel {

struct script_node_type;
struct script_thread_type;

/** \brief The type used to store evaluated expression within the script engine.
 *
 * `void` is stored as an `h_long` with value `0`.
 */
union script_value_union {
    identity<> u_id;
    identity<script_node_type> u_script_node;

    boolean    u_boolean;
    real       u_real;
    h_short    u_short;
    h_long     u_long;
    h_ccstr    u_string;
};

/** \brief Definition for a function as exposed to the script engine.
 */
struct script_function {
    /** \brief Parses the expression referring to a call to this function.
     *
     * \param[in] function_index The index of the function in the procedures table.
     * \param[in] script_node_id The identity of the script node.
     * \return Non-zero on success, otherwise `0`.
     */
    using parse_tproc __attribute__((cdecl))
        = boolean(*)(index_short                function_index,
                     identity<script_node_type> script_node_id);

    /** \brief Performs a step in the evaluation of a call to this function.
     *
     * This is not a true evaluation function.
     * If evaluation occurred in a single step, then the OS thread's stack would be
     * expanded to account for each nested evaluation and may overflow.
     *
     * Instead, this coroutine function pushes frames onto the scripting thread's
     * local stack that are required for evaluation of the function and returns
     * control back to the scripting engine.
     * The scripting engine will then loop back over and evaluate the frame at the
     * top of the stack.
     * This process repeats the script thread is terminated or suspended.
     *
     * \param[in] function_index   The index of the function in the procedures table.
     * \param[in] thread_id        The identity of the script thread of execution.
     * \param[in] initialize_frame Non-zero if the top frame should be initialized.
     */
    using coroutine_tproc __attribute__((cdecl))
        = void(*)(index_short                  function_index,
                  identity<script_thread_type> thread_id,
                  boolean                      initialize_frame);

    enum_short      return_type;
    h_ccstr         name;
    parse_tproc     parse;
    coroutine_tproc co_eval;
    h_ccstr         help_usage;
    h_ccstr         help_parameters;
    mask_byte       access_requirements; // different between PC and CE, always a byte
    h_short         param_count;
    enum_short      param_types[]; // flexible array, required by Halo's functions
}; static_assert(sizeof(script_function) == 0x1C);

/** \brief Represents a node in the parse tree.
 */
struct script_node_type {
    identity_salt salt;       // 00
    h_short       unknown0;   // 02, function index if it is a call expression
    enum_short    value_type; ///< 0: unparsed, 4: void, 5: bool, 6: real,
                              ///< 7: short, 8: long, 9: string, 10: script
    flags_short   flags;      // 06
    identity<script_node_type> next_node; // 08
    h_long        position;   // 0C, locates the expression in source i think,
                              // ive seen it offset from [scenario_tag + 4A0]

    script_value_union value; // 10
}; static_assert(sizeof(script_node_type) == 0x14);

/** \brief Stack frames in a scripting thread.
 */
struct script_frame_type {
    script_frame_type* previous;       // 00
    identity<script_node_type> script_node; // 04
    script_value_union* result;        // 08, available in call expression frame
                                       //     (but not the invocation context itself)
    h_short            size;           // 0C, size of array data in bytes
    char               data[];         // 0E, flexible array, required for Halo
}; static_assert(sizeof(script_frame_type) == 0x10);

/** \brief Storage for the context of a flow of execution in the scripting engine.
 */
struct script_thread_type {
    identity_salt      salt;      // 00
    uint8              unknown0;  // 02
    flags_byte         unknown1;  // 03, LSB gets cleared by mask FE during update
                                  // LSB set means the top frame is uninitialized
    uint32             unknown2;  // 04
    ticks_long         wake_tick; ///< the tick to wake up on, with respect to
                                  ///< pGameTimeGlobals->game_ticks
    int32              unknown3;  // 0C
    script_frame_type* frame;     // 10, initialized to &data, top frame on stack
    uint32             unknown4;  // 14
    char               data[512]; // 18
}; static_assert(sizeof(script_thread_type) == 0x218);

/** \brief Used to assist symbol lookup, parsing, and a bit of evaluation.
 */
struct user_evaluation_buffer_type {
    h_short   capacity;          ///< The capacity of the array #candidates.
    h_ccstr   lookup;            ///< The name to lookup. This is never `nullptr`.
    boolean   recompile_scripts; // set by (script_recompile)
    mask_long access_mask;       // different semantics on PC compared to CE
                                 // cast to mask_short on PC
                                 // cast to index_ubyte on CE
    int16     candidates_count;  ///< The number of candidates written to #candidates.
    h_ccstr*  candidates;        ///< The candidates for #lookup
    boolean   in_use;            ///< Never read from

    h_long    script_data_end; // eval only, 1-past-end fashion
    h_ccstr   script_data;     // eval only, the expression is appended to this during
                              // evaluation probably for (script_recompile)
}; static_assert(sizeof(user_evaluation_buffer_type) == 0x24);

} // namespace sentinel
