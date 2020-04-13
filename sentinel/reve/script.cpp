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

std::vector<const reve::script::script_function*> script_functions;

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

    constexpr auto max_number_functions = std::numeric_limits<h_short>::max();
    if (max_number_functions <= index) {
        std::fprintf(stderr, "no room for user function\n");
        return nullptr;
    }

    bool insert_at_end = insert_pos == script_functions.end();
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

    if (handle && pFunctionIndex)
        *pFunctionIndex = index;
    return handle;
}

bool Init()
{
    switch (edition) {
    case GameEdition::combat_evolved: ScriptFunctionsArraySize = 0x20A; break;
    case GameEdition::custom_edition: ScriptFunctionsArraySize = 0x211; break;
    }

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

#define DEFINE_PROCEDURE_INDEXER_HANDLER2(dst, index, save1, save2)    \
void procedure_index_handler_##dst##_##index() __attribute__((naked)); \
void procedure_index_handler_##dst##_##index()                         \
{                                                                      \
    asm("pushfl \n\t"                                                  \
        "pushl %" #save1 " \n\t"                                       \
        "pushl %" #save2 " \n\t"                                       \
                                                                       \
        "pushl %" #index " \n\t"                                       \
        "call _sentinel_script_get_function_by_index \n\t"             \
        "add $4, %esp \n\t"                                            \
        "mov %eax, %" #dst " \n\t"                                     \
                                                                       \
        "popl %" #save2 " \n\t"                                        \
        "popl %" #save1 " \n\t"                                        \
        "popfl \n\t"                                                   \
        "ret \n\t");                                                   \
}

#define DEFINE_PROCEDURE_INDEXER_HANDLER3(dst, index, save1, save2, save3) \
void procedure_index_handler_##dst##_##index() __attribute__((naked));     \
void procedure_index_handler_##dst##_##index()                             \
{                                                                          \
    asm("pushfl \n\t"                                                      \
        "pushl %" #save1 " \n\t"                                           \
        "pushl %" #save2 " \n\t"                                           \
        "pushl %" #save3 " \n\t"                                           \
                                                                           \
        "pushl %" #index " \n\t"                                           \
        "call _sentinel_script_get_function_by_index \n\t"                 \
        "add $4, %esp \n\t"                                                \
        "mov %eax, %" #dst " \n\t"                                         \
                                                                           \
        "popl %" #save3 " \n\t"                                            \
        "popl %" #save2 " \n\t"                                            \
        "popl %" #save1 " \n\t"                                            \
        "popfl \n\t"                                                       \
        "ret \n\t");                                                       \
}

#define DEFINE_MOV_PROCEDURES_OFFSET_HANDLER(dst)                  \
void mov_procedures_offset_handler_##dst() __attribute__((naked)); \
void mov_procedures_offset_handler_##dst()                         \
{                                                                  \
    asm("pushfl \n\t"                                              \
        "pushl %eax \n\t"                                          \
        "pushl %edx \n\t"                                          \
        "pushl %ecx \n\t"                                          \
                                                                   \
        "call _sentinel_script_get_functions_array \n\t"           \
        "movl %eax, %" #dst " \n\t"                                \
                                                                   \
        "popl %ecx \n\t"                                           \
        "popl %edx \n\t"                                           \
        "popl %eax \n\t"                                           \
        "popfl \n\t"                                               \
        "ret \n\t");                                               \
}

// 8B0485 588B6800  MOV EAX,DWORD PTR DS:[EAX*4+ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(eax, eax, edx, ecx)

// 8B0495 588B6800  MOV EAX,DWORD PTR DS:[EDX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(eax, edx, edx, ecx)

// 8B048D 588B6800  MOV EAX,DWORD PTR DS:[ECX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(eax, ecx, edx, ecx)

// 8B148D 588B6800  |MOV EDX,DWORD PTR DS:[ECX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(edx, ecx, eax, ecx)

// 8B1495 588B6800  MOV EDX,DWORD PTR DS:[EDX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(edx, edx, eax, ecx)

// 8B0C85 588B6800  MOV ECX,DWORD PTR DS:[EAX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER2(ecx, eax, eax, edx)

// 8B1C85 588B6800  MOV EBX,DWORD PTR DS:[EAX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER3(ebx, eax, eax, edx, ecx)

// 8B2C85 588B6800  MOV EBP,DWORD PTR DS:[EAX*4+script::ScriptFunctionArray]
DEFINE_PROCEDURE_INDEXER_HANDLER3(ebp, eax, eax, edx, ecx)

// BB 588B6800   MOV EBX,OFFSET script::ScriptFunctionsArray
DEFINE_MOV_PROCEDURES_OFFSET_HANDLER(ebx)

// BD 588B6800   MOV EBP,OFFSET script::ScriptFunctionsArray
DEFINE_MOV_PROCEDURES_OFFSET_HANDLER(ebp)

// 66:81FE 0A02        |CMP SI,20A
void cmp_procedure_counter_handler_si() __attribute__((naked));
void cmp_procedure_counter_handler_si()
{
    asm("pushl %eax \n\t"
        "pushl %edx \n\t"
        "pushl %ecx \n\t"

        "call _sentinel_script_get_functions_array_size \n\t"
        "cmp %ax, %si \n\t"

        "popl %ecx \n\t"
        "popl %edx \n\t"
        "popl %eax \n\t"
        "ret \n\t");
}

// C74424 10 0A020000  MOV DWORD PTR SS:[ESP+10],20A
void mov_procedure_count_to_local_handler() __attribute__((naked));
void mov_procedure_count_to_local_handler()
{
    asm("pushl %eax \n\t"
        "pushl %edx \n\t"
        "pushl %ecx \n\t"

        "call _sentinel_script_get_functions_array_size \n\t"
        "movsx %ax, %eax \n\t"
        "movl %eax, 0x20(%esp) \n\t"

        "popl %ecx \n\t"
        "popl %edx \n\t"
        "popl %eax \n\t"
        "ret \n\t");
}

// BD 11020000         MOV EBP,211
void mov_procedure_count_to_ebp_handler() __attribute__((naked));
void mov_procedure_count_to_ebp_handler()
{
    asm("pushl %eax \n\t"
        "pushl %edx \n\t"
        "pushl %ecx \n\t"

        "call _sentinel_script_get_functions_array_size \n\t"
        "movsx %ax, %eax \n\t"
        "movl %eax, %ebp \n\t"

        "popl %ecx \n\t"
        "popl %edx \n\t"
        "popl %eax \n\t"
        "ret \n\t");
}

#undef DEFINE_PROCEDURE_INDEXER_HANDLER2
#undef DEFINE_PROCEDURE_INDEXER_HANDLER3
#undef DEFINE_MOV_PROCEDURES_OFFSET_HANDLER

bool PerformDeepProcedureHook()
{
    using reve::script::ptr_ScriptFunctionsArray;
    using detours::patch;
    using detours::descriptors::detour_call;
    using detours::descriptors::bytes;
    using detours::descriptors::range_descriptor;
    using detours::descriptors::imbued_descriptor;
    using detours::get_detour_data;

    std::unordered_map<std::uint32_t, void*> indexing_replacement_handlers {
        {0x8B0485u, (void*)&procedure_index_handler_eax_eax},
        {0x8B0495u, (void*)&procedure_index_handler_eax_edx},
        {0x8B048Du, (void*)&procedure_index_handler_eax_ecx},
        {0x8B148Du, (void*)&procedure_index_handler_edx_ecx},
        {0x8B1495u, (void*)&procedure_index_handler_edx_edx},
        {0x8B0C85u, (void*)&procedure_index_handler_ecx_eax},
        {0x8B1C85u, (void*)&procedure_index_handler_ebx_eax},
        {0x8B2C85u, (void*)&procedure_index_handler_ebp_eax},
    };

    std::unordered_map<std::uint32_t, void*> offset_replacement_handles {
        {0xBBu, (void*)&mov_procedures_offset_handler_ebx},
        {0xBDu, (void*)&mov_procedures_offset_handler_ebp},
    };

    auto array_addr
        = detours::as_byte_array((std::uint32_t)ptr_ScriptFunctionsArray);
    range_descriptor index_script_function_descriptor {
        imbued_descriptor {
            bytes{0x8B, -1, -1,
                  array_addr[0], array_addr[1], array_addr[2], array_addr[3]},
            [&indexing_replacement_handlers] (unsigned char* site, auto patch_out)
                -> bool
            {
                auto inst = site[0] << 16 | site[1] << 8 | site[2];
                if (!indexing_replacement_handlers.count(inst)) {
                    SENTINEL_DEBUG_MESSAGE("at %p, unhandled inst: %x\n", site, inst);
                    return false;
                }

                void* handler = indexing_replacement_handlers.at(inst);

                patch p(site, get_detour_data<2>(detour_call, site, handler));
                if (p) {
                    *patch_out++ = std::move(p);
                    return true;
                }

                return false;
            }
        }
    };

    range_descriptor mov_script_functions_offset_descriptor {
        imbued_descriptor {
            bytes{-1, array_addr[0], array_addr[1], array_addr[2], array_addr[3]},
            [&offset_replacement_handles] (unsigned char* site, auto patch_out)
                -> bool
            {
                auto inst = site[0];
                if (!offset_replacement_handles.count(inst)) {
                    SENTINEL_DEBUG_MESSAGE("at %p, unhandled inst: %x\n", site, inst);
                    return false;
                }

                void* handler = offset_replacement_handles.at(inst);

                patch p(site, get_detour_data(detour_call, site, handler));
                if (p) {
                    *patch_out++ = std::move(p);
                    return true;
                }

                return false;
            }
        }
    };

    auto num_functions
        = detours::as_byte_array(reve::script::ScriptFunctionsArraySize);
    range_descriptor cmp_procedure_counter_descriptor {
        imbued_descriptor {
            bytes{0x66, 0x81, 0xFE, num_functions[0], num_functions[1]},
            [] (unsigned char* site, auto patch_out) -> bool
            {
                auto handler = &cmp_procedure_counter_handler_si;
                patch p(site, get_detour_data(detour_call, site, handler));
                if (p) {
                    *patch_out++ = std::move(p);
                    return true;
                }
                return false;
            }
        }
    };

    range_descriptor mov_procedure_count_to_local_descriptor {
        imbued_descriptor {
            bytes{0xC7, 0x44, 0x24, 0x10, 0x0A, 0x02, 0x00, 0x00},
            [] (unsigned char* site, auto patch_out) -> bool
            {
                auto handler = &mov_procedure_count_to_local_handler;
                patch p(site, get_detour_data<3>(detour_call, site, handler));
                if (p) {
                    *patch_out++ = std::move(p);
                    return true;
                }
                return false;
            }
        }
    };

    range_descriptor mov_procedure_count_to_ebp_descriptor {
        imbued_descriptor {
            bytes{0xBD, 0x11, 0x02, 0x00, 0x00},
            [] (unsigned char* site, auto patch_out) -> bool
            {
                auto handler = &mov_procedure_count_to_ebp_handler;
                patch p(site, get_detour_data(detour_call, site, handler));
                if (p) {
                    *patch_out++ = std::move(p);
                    return true;
                }
                return false;
            }
        }
    };

    if (!detours::make_patch(index_script_function_descriptor) ||
        !detours::make_patch(mov_script_functions_offset_descriptor) ||
        !detours::make_patch(cmp_procedure_counter_descriptor) ||
        !(detours::make_patch(mov_procedure_count_to_local_descriptor) ||
          detours::make_patch(mov_procedure_count_to_ebp_descriptor))) {
        SENTINEL_DEBUG_MESSAGE("deep hooks for script engine were unsuccessful\n");
        return false;
    }

    return true;
}

} // namespace (anonymous)
