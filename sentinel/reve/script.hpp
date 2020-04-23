
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstddef>

#include <array>
#include <type_traits>

#include "types.hpp"

#include <sentinel/base.hpp>
#include <sentinel/structures/script.hpp>

namespace reve { namespace script {

using sentinel::script_function;
using sentinel::script_value_union;
using sentinel::script_thread_type;
using sentinel::user_evaluation_buffer_type;

/** \brief Performs a lookup using the data set in \ref ptr_SymbolLookupBuffer.
 *
 * \sa ptr_SymbolLookupProcedures
 */
using symbol_lookup_tproc __attribute__((cdecl)) = void(*)();

/** \brief Processes \a expression as if it was entered into the console.
 *
 * A script node is temporarily created to house the expression, but the result of
 * the expression is discarded. Hence, this is not an evaluation method.
 *
 * \return Non-zero on success, or `0` on failure.
 */
using process_expression_tproc __attribute__((cdecl))
    = bool8(*)(P_IN h_ccstr expression /*STACK*/);

/** \brief Returns from the call context at the thread's top stack frame.
 *
 * \param[in] return_value The value to return from the call.
 * \param[in] thread_id    The identity of the thread.
 */
using function_context_return_tproc __attribute__((cdecl, regparm(3)))
    = void(*)(P_IN script_value_union return_value /*EAX*/,
              P_IN regint           /*dummy*/      /*EDX*/,
              P_IN identity_raw       thread_id    /*ECX*/);

/** \brief Parses the script node by \a id with an expected value \a type.
 *
 * \param[in] id The identity of the script node to parse.
 *               This must be a valid identity.
 * \param[in] type The expected result type of the script node's expression.
 * \return Non-zero if the script node could be parsed to the expected value \a type,
 *         otherwise `0`.
 */
using parse_script_node_expected_tproc __attribute__((cdecl))
    = bool8(*)(P_IN identity_raw id,
               P_IN enum_short   type);

/** \brief Pushes a new frame onto a script thread's stack which evaluates an
 *         expression and stores the result in \a value.
 *
 * \param[in]  script_node_id The identity of the expression.
 * \param[in]  thread_id      The identity of the thread.
 * \param[out] value          Receives the value of evaluating the expression.
 *
 * \sa threadPushEvalFrame
 */
using thread_push_eval_frame_tproc
    = exotic_function<void(P_IN  identity_raw        script_node_id /*EAX*/,
                           P_IN  identity_raw        thread_id      /*EDX*/,
                           P_OUT script_value_union* value          /*EBX*/)>;

/** \brief An array of procedures that fill in the \ref ptr_SymbolLookupBuffer
 *         with candidates, with different procedures referring to different
 *         symbol types.
 *
 * \sa proc_LookupBasicCommands
 *     proc_LookupGlobals
 */
extern std::array<symbol_lookup_tproc, 18>* ptr_SymbolLookupProcedures;
extern user_evaluation_buffer_type*         ptr_UserEvaluationBuffer;

extern script_function** ptr_ScriptFunctionsArray;
extern h_short           ScriptFunctionsArraySize;

extern symbol_lookup_tproc      proc_LookupBasicCommands;
extern symbol_lookup_tproc      proc_LookupGlobals;
extern process_expression_tproc proc_ProcessExpression;

extern function_context_return_tproc    proc_FunctionContextReturn;
extern parse_script_node_expected_tproc proc_ParseScriptNodeExpected;
extern thread_push_eval_frame_tproc     proc_PushEvalFrame;

extern script_function::parse_tproc proc_ParseFromDefinition;

/** \brief Interface for the exotic function \ref thread_push_eval_frame_tproc.
 */
void ThreadPushEvalFrame(identity_raw        script_node_id,
                         identity_raw        thread_id,
                         script_value_union* value);

/** \brief Installs a \a function into the scripting engine.
 *
 * Unlike other installations, \a function must outlive the installation.
 *
 * \param[in]  function       A pointer to the function to install.
 * \param[out] function_index (Nullable) Receives the index of the installed function.
 * \return A handle to the installation, or `nullptr` on failure.
 */
sentinel_handle InstallScriptFunction(const script_function* function,
                                      int* pFunctionIndex);

bool Init();

void Debug();

} } // namespace reve::script

