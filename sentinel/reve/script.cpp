
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "script.hpp"
#include "globals.hpp"

#include <cstdio>

#include <algorithm>     // std::find_if
#include <limits>        // std::numeric_limits
#include <unordered_map> // std::unordered_map
#include <utility>       // std::move
#include <string_view>   // std::string_view

#include <detours/detours.hpp>
#include <sentinel/script.hpp>

namespace {

constexpr auto maximum_script_functions = std::numeric_limits<sentinel::h_short>::max();

std::vector<const reve::script::script_function*> script_functions;
std::vector<unsigned char*> script_functions_array_references;
std::vector<unsigned char*> script_functions_array_count_references;      // as h_short
std::vector<unsigned char*> script_functions_array_long_count_references; // as h_long
std::vector<detours::patch> script_functions_array_patches;

extern "C"
const reve::script::script_function**
sentinel_script_get_functions_array() __attribute__((cdecl));

extern "C"
std::int16_t
sentinel_script_get_functions_array_size() __attribute__((cdecl));

extern "C"
const reve::script::script_function*
sentinel_script_get_function_by_index(reve::regint index) __attribute__((cdecl));

const reve::script::script_function* LookupScriptFunction(std::string_view name);

void SetScriptEngineFunctionsExports();

bool PerformDeepProcedureHook();

bool InvalidateScriptFunctionsArrayReferences();

} // namespace (anonymous)

namespace reve { namespace script {

std::array<symbol_lookup_tproc, 18>* ptr_SymbolLookupProcedures = nullptr;
user_evaluation_buffer_type*         ptr_UserEvaluationBuffer = nullptr;

script_function** ptr_ScriptFunctionsArray = nullptr;
h_short           ScriptFunctionsArraySize = 0;

symbol_lookup_tproc      proc_LookupBasicCommands = nullptr;
symbol_lookup_tproc      proc_LookupGlobals       = nullptr;
process_expression_tproc proc_ProcessExpression   = nullptr;

function_context_return_tproc    proc_FunctionContextReturn   = nullptr;
parse_script_node_expected_tproc proc_ParseScriptNodeExpected = nullptr;
thread_push_eval_frame_tproc     proc_PushEvalFrame           = nullptr;

script_function::parse_tproc proc_ParseFromDefinition = nullptr;

void ThreadPushEvalFrame(identity_raw        script_node_id,
                         identity_raw        thread_id,
                         script_value_union* value)
{
    regint discard;
    asm volatile("CALL *%[PushEvalFrame] \n\t"
        : "=a" (discard), "=d" (discard), "=c" (discard), "=m" (*value)
        : [PushEvalFrame] "rm" (proc_PushEvalFrame), "a" (script_node_id),
                 "d" (thread_id), "b" (value)
        : "memory", "cc");
}

sentinel_handle InstallScriptFunction(const script_function* function,
                                      int* pFunctionIndex)
{
    static constexpr reve::script::script_function empty_function
    { 4, "<expired user function>", nullptr, nullptr, nullptr, nullptr, 0, 0 };

    if (function == nullptr)
        return nullptr;

    // the element to overwrite, or otherwise the end iterator, to insert at the end
    auto insert_pos = std::find_if(script_functions.begin(),
                                   script_functions.end(),
                                   [] (auto f) { return f == &empty_function; });
    const auto index = insert_pos - script_functions.cbegin();

    if (index >= maximum_script_functions) {
        std::fprintf(stderr, "no room for user function\n");
        return nullptr;
    }

    const bool insert_at_end = insert_pos == script_functions.end();
    //const bool need_invalidation = insert_at_end && script_functions.size() == script_functions.capacity();
    bool function_added = false;
    sentinel_handle handle = [&] {
        try {
            if (insert_at_end) script_functions.push_back(function);
            else               *insert_pos = function;
            function_added = true;

            [[maybe_unused]]
            auto uninstall_cb = [function, index] (auto) mutable
            {
                script_functions[index] = &empty_function;
            };
            return sentinel::callback_handle(std::move(uninstall_cb));
        } catch (...) {
            std::fprintf(stderr, "failed to insert user function\n");
            if (function_added) {
                if (insert_at_end) script_functions.pop_back();
                else               script_functions[index] = &empty_function;
            }
            return (sentinel_handle)nullptr;
        }
    }();

    InvalidateScriptFunctionsArrayReferences();

    if (handle && pFunctionIndex)
        *pFunctionIndex = index;
    return handle;
}

bool Init()
{
    if (!ptr_SymbolLookupProcedures)
        return false;

    proc_LookupBasicCommands = (*ptr_SymbolLookupProcedures)[3];
    proc_LookupGlobals       = (*ptr_SymbolLookupProcedures)[5];

    if (ptr_ScriptFunctionsArray)
        script_functions.insert(script_functions.cend(),
                                ptr_ScriptFunctionsArray,
                                ptr_ScriptFunctionsArray + ScriptFunctionsArraySize);

    if (auto object_destroy_def = LookupScriptFunction("object_destroy"))
        proc_ParseFromDefinition = object_destroy_def->parse;

    SetScriptEngineFunctionsExports();

    return ptr_UserEvaluationBuffer
        && ptr_SymbolLookupProcedures
        && ptr_ScriptFunctionsArray
        && ScriptFunctionsArraySize
        && proc_LookupBasicCommands
        && proc_LookupGlobals
        && proc_ProcessExpression
        && proc_FunctionContextReturn
        && proc_ParseScriptNodeExpected
        && proc_PushEvalFrame
        && proc_ParseFromDefinition
        && PerformDeepProcedureHook();
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", ptr_UserEvaluationBuffer);
    SENTINEL_DEBUG_VAR("%p", ptr_SymbolLookupProcedures);
    SENTINEL_DEBUG_VAR("%p", ptr_ScriptFunctionsArray);
    SENTINEL_DEBUG_VAR("%ld", (long)ScriptFunctionsArraySize);
    SENTINEL_DEBUG_VAR("%p", proc_LookupBasicCommands);
    SENTINEL_DEBUG_VAR("%p", proc_LookupGlobals);
    SENTINEL_DEBUG_VAR("%p", proc_ProcessExpression);
    SENTINEL_DEBUG_VAR("%p", proc_FunctionContextReturn);
    SENTINEL_DEBUG_VAR("%p", proc_ParseScriptNodeExpected);
    SENTINEL_DEBUG_VAR("%p", proc_PushEvalFrame);
    SENTINEL_DEBUG_VAR("%p", proc_ParseFromDefinition);
}

} } // namespace reve::script

namespace {

extern "C"
const reve::script::script_function**
sentinel_script_get_functions_array()
{ return script_functions.data(); }

extern "C"
std::int16_t
sentinel_script_get_functions_array_size()
{ return static_cast<std::int16_t>(script_functions.size()); }

extern "C"
const reve::script::script_function*
sentinel_script_get_function_by_index(reve::regint index)
{ return sentinel_script_get_functions_array()[index]; }

void SetScriptEngineFunctionsExports()
{
    using sentinel::index_short;
    using sentinel::enum_short;
    using sentinel::script_engine_functions_aggregate;
    using sentinel::identity;
    using sentinel::script_value_union;
    using sentinel::script_node_type;
    using sentinel::script_thread_type;

    using reve::script::proc_ParseFromDefinition;
    using reve::script::proc_ParseScriptNodeExpected;
    using reve::script::proc_FunctionContextReturn;
    using reve::script::ThreadPushEvalFrame;

    sentinel_script_ScriptEngineFunctions = {
        proc_ParseFromDefinition,
        +[] (identity<script_node_type> script_node,
             enum_short type) -> bool
            { return proc_ParseScriptNodeExpected(script_node.raw, type); },
        +[] (script_value_union retval,
             identity<script_thread_type> thread) -> void
            { return proc_FunctionContextReturn(retval, 0, thread.raw); },
        +[] (identity<script_node_type> script_node,
             identity<script_thread_type> thread,
             script_value_union* value) -> void
            { return ThreadPushEvalFrame(script_node.raw, thread.raw, value); }
    };
}

const reve::script::script_function* LookupScriptFunction(std::string_view name)
{
    auto pDef = std::find_if(script_functions.cbegin(),
                             script_functions.cend(),
                             [name] (auto p) { return name == p->name; });

    if (pDef == script_functions.cend())
        return nullptr;
    return *pDef;
}

bool PerformDeepProcedureHook()
{
    using reve::script::ptr_ScriptFunctionsArray;
    using reve::script::ScriptFunctionsArraySize;
    using detours::descriptors::bytes;
    using detours::descriptors::range_descriptor;
    using detours::descriptors::imbued_descriptor;

    auto array_bytes       = detours::as_byte_array((std::uint32_t)ptr_ScriptFunctionsArray);
    auto array_count_bytes = detours::as_byte_array((std::int32_t)reve::script::ScriptFunctionsArraySize);

    // MOV EDX,DWORD PTR DS:[ECX*4+script::ScriptFunctionsArray]; 8B 14 8D <>
    // MOV EBP,DWORD PTR DS:[EAX*4+script::ScriptFunctionsArray]; 8B 2C 85 <>
    // MOV EAX,DWORD PTR DS:[ECX*4+script::ScriptFunctionsArray]; 8B 04 8D <>
    // MOV EDX,DWORD PTR DS:[EDX*4+script::ScriptFunctionsArray]; 8B 14 95 <>
    // MOV EBX,DWORD PTR DS:[EAX*4+script::ScriptFunctionsArray]; 8B 1C 85 <>
    range_descriptor index_script_function_descriptor { imbued_descriptor {
        bytes{0x8B, -1, -1, array_bytes[0], array_bytes[1], array_bytes[2], array_bytes[3]},
        [] (unsigned char* site, auto /*unused*/) {
            if ((site[1] == 0x14 && site[2] == 0x8D) ||
                (site[1] == 0x2C && site[2] == 0x85) ||
                (site[1] == 0x04 && site[2] == 0x8D) ||
                (site[1] == 0x14 && site[2] == 0x95) ||
                (site[1] == 0x1C && site[2] == 0x85))
                script_functions_array_references.push_back(site + 3);
            return true;
        }
    } };

    // MOV EBP,OFFSET script::ScriptFunctionsArray; BD <>
    // MOV EBX,OFFSET script::ScriptFunctionsArray; BB <>
    range_descriptor mov_script_functions_offset_descriptor { imbued_descriptor {
        bytes{-1, array_bytes[0], array_bytes[1], array_bytes[2], array_bytes[3]},
        [] (unsigned char* site, auto /*unused*/) {
            if (site[0] == 0xBD || site[0] == 0xBB)
                script_functions_array_references.push_back(site + 1);
            return true;
        }
    } };

    // CMP SI, *; (0x20A on PC, 0x211 on CE) signed semantics
    range_descriptor cmp_functions_array_size_descriptor { imbued_descriptor {
        bytes{0x66, 0x81, 0xFE, array_count_bytes[0], array_count_bytes[1]},
        [] (unsigned char* site, auto) {
            script_functions_array_count_references.push_back(site + 3);
            return true;
        }
    } };

    // MOV DWORD PTR SS:[ESP+10],20A (specific to PC)
    range_descriptor mov_functions_array_count_to_local_descriptor { imbued_descriptor {
        bytes{0xC7, 0x44, 0x24, 0x10, array_count_bytes[0], array_count_bytes[1], array_count_bytes[2], array_count_bytes[3]},
        [] (unsigned char* site, auto) {
            script_functions_array_long_count_references.push_back(site + 4);
            return true;
        }
    } };

    // MOV EBP, 211 (specific to CE, counterpart to the above descriptor)
    range_descriptor mov_functions_array_count_to_ebp_descriptor { imbued_descriptor {
        bytes{0xBD, array_count_bytes[0], array_count_bytes[1], array_count_bytes[2], array_count_bytes[3]},
        [] (unsigned char* site, auto) {
            script_functions_array_long_count_references.push_back(site + 1);
            return true;
        }
    } };

    detours::make_patch(index_script_function_descriptor);
    detours::make_patch(mov_script_functions_offset_descriptor);
    detours::make_patch(cmp_functions_array_size_descriptor);
    detours::make_patch(mov_functions_array_count_to_local_descriptor);
    detours::make_patch(mov_functions_array_count_to_ebp_descriptor);

    if (script_functions_array_long_count_references.size() != 1 ||
        script_functions_array_count_references.size() != 2 ||
        script_functions_array_references.size() != 10 ||
        !InvalidateScriptFunctionsArrayReferences()) {
        SENTINEL_DEBUG_MESSAGE("deep hooks for script engine were unsuccessful\n");
        return false;
    }

    return true;
}

bool InvalidateScriptFunctionsArrayReferences()
{
    if (script_functions.size() > maximum_script_functions)
        return false;

    script_functions_array_patches.clear();
    {
        auto count_bytes = detours::as_byte_array(static_cast<sentinel::h_long>(script_functions.size()));
        for (auto long_count_site : script_functions_array_long_count_references)
            script_functions_array_patches.emplace_back(long_count_site, count_bytes);
    }

    {
        auto count_bytes = detours::as_byte_array(static_cast<sentinel::h_short>(script_functions.size()));
        for (auto short_count_site : script_functions_array_count_references)
            script_functions_array_patches.emplace_back(short_count_site, count_bytes);
    }

    {
        auto array_bytes = detours::as_byte_array((std::uint32_t)script_functions.data());
        for (auto array_site : script_functions_array_references)
            script_functions_array_patches.emplace_back(array_site, array_bytes);
    }

    return std::all_of(script_functions_array_patches.cbegin(),
                       script_functions_array_patches.cend(),
                       [] (auto& patch) { return static_cast<bool>(patch); });
}

} // namespace (anonymous)
