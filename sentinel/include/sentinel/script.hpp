
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/base.hpp>
#include <sentinel/structures/script.hpp>

namespace sentinel {

struct script_engine_functions_aggregate {
    using parse_tproc = script_function::parse_tproc;
    using parse_script_node_tproc = bool(*)(identity<script_node_type> script_node,
                                            enum_short type);
    using context_return_tproc  = void(*)(script_value_union retval,
                                          identity<script_thread_type> thread);
    using push_eval_frame_tproc = void(*)(identity<script_node_type> script_node,
                                          identity<script_thread_type> thread,
                                          script_value_union* destination);

    /** \brief A basic parsing function that uses the function definition to parse
     *         calls to the function.
     */
    parse_tproc parse_from_definition;

    /** \brief Parses a script node with an expected value type, returning `true`
     *         to indicate success and `false` to indicate failure.
     */
    parse_script_node_tproc parse_expected;

    /** \brief Causes a thread to return from a function body frame on the top of
     *         its stack with a specific value.
     */
    context_return_tproc return_from_context;

    /** \brief Pushes a frame onto a thread's stack which evaluates a given expression
     *         and outputs the value into a supplied destination.
     */
    push_eval_frame_tproc push_evaluation_frame;
};

} // namespace sentinel

extern "C" {

/** \brief Installs \a function into the script engine.
 *
 * \a function must outlive the installation.
 *
 * Both \a free_fn and \a function_index are nullable.
 *
 * \param[in]  function The function to install.
 * \param[out] function_index Receives the index of the installed function.
 *
 * \return A handle to the installation, or `nullptr` if it failed to install.
 */
SENTINEL_API
sentinel_handle
sentinel_script_InstallFunction(const sentinel::script_function* function,
                                int* function_index);

extern
SENTINEL_API
sentinel::script_engine_functions_aggregate
sentinel_script_ScriptEngineFunctions;

} // extern "C""
