
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "table.hpp"

#include <cstdio>

#include <algorithm>   // std::find_if
#include <array>       // std::array
#include <string_view> // std::string_view
#include <utility>     // std::pair

#include <sentinel/globals.hpp>

/* hook_TableCreateFrom* hooks are trampolines (see sentinel_handler_TableCreate).
   Although it would not be difficult to write replacement functions rather than use
   a trampoline, writing a replacement function might break other libraries that load
   similarly to sentinel.
 */

namespace {

using namespace std::literals::string_view_literals;
inline sentinel::table_aggregate_type& tables = sentinel_Globals_tables;

const std::array table_watches {
    std::pair{"effect"sv, (void**)&tables.effect_table},
    std::pair{"object"sv, (void**)&tables.object_table},
    std::pair{"players"sv, (void**)&tables.player_table},
    std::pair{"terminal output"sv, (void**)&tables.terminal_output_table},
    std::pair{"script node"sv, (void**)&tables.script_node_table},
    std::pair{"hs thread"sv, (void**)&tables.script_thread_table}
};

void record_table(std::string_view name, void* table);
void delete_table(void* table);

} // namespace (anonymous)

namespace reve { namespace table {

detours::meta_patch patch_CreateTableFromAllocator;
detours::meta_patch patch_CreateTableFromHeap;

sentinel::table_type<sentinel::player>* ptr_PlayersTable = nullptr;

remove_table_element_tproc proc_RemoveTableElement       = nullptr;
create_table_tproc         proc_CreateTableFromAllocator = nullptr;
create_table_tproc         proc_CreateTableFromHeap      = nullptr;


extern "C"
void*
sentinel_handler_CreateTable(int32 to_heap,
                             int16 datum_size,
                             h_ccstr szName,
                             int16 datum_count)
{
    auto& proc = to_heap ? proc_CreateTableFromHeap
                         : proc_CreateTableFromAllocator;
    auto& patch = to_heap ? patch_CreateTableFromHeap
                          : patch_CreateTableFromAllocator;

    // trampoline
    patch.restore();
    void* lpTable;
    asm("pushl %4 \n\t"
        "pushl %3 \n\t"
        "call *%1 \n\t"
        "add $0x08, %%esp \n\t"
        : "=a" (lpTable)
        : "rm" (proc), "b" ((uint32)datum_size), "r" (szName),
          "r" ((uint32)datum_count)
        : "cc", "edx", "ecx");
    patch.repatch();

    record_table(szName, lpTable);

    return lpTable;
}

void*
tramp_CreateTableFromAllocator(/*P_IN int16       datum_size  [EBX],
                                 P_IN h_ccstr     szName      [stack],
                                 P_IN int16       datum_count [stack]*/)
{
    asm("movl 8(%esp), %edx \n\t"  // EDX <- datum_count
        "movl 4(%esp), %eax \n\t"  // EAX <- szName
        "pushl %edx \n\t"
        "pushl %eax \n\t"
        "pushl %ebx \n\t"
        "pushl $0 \n\t"
        "call _sentinel_handler_CreateTable \n\t"
        "add $0x10, %esp \n\t"
        "ret \n\t");
}

void*
tramp_CreateTableFromHeap(/*P_IN int16       datum_size  [EBX],
                            P_IN h_ccstr     szName      [stack],
                            P_IN int16       datum_count [stack]*/)
{
    asm("movl 8(%esp), %edx \n\t"  // EDX <- datum_count
        "movl 4(%esp), %eax \n\t"  // EAX <- szName
        "pushl %edx \n\t"
        "pushl %eax \n\t"
        "pushl %ebx \n\t"
        "pushl $1 \n\t"
        "call _sentinel_handler_CreateTable \n\t"
        "add $0x10, %esp \n\t"
        "ret \n\t");
}

void SignalTableDestructing(void* table) { delete_table(table); }

bool Init()
{
    return proc_RemoveTableElement
        && proc_CreateTableFromAllocator
        && proc_CreateTableFromHeap
        && patch_CreateTableFromAllocator
        && patch_CreateTableFromHeap;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_RemoveTableElement);
    SENTINEL_DEBUG_VAR("%p", proc_CreateTableFromAllocator);
    SENTINEL_DEBUG_VAR("%p", proc_CreateTableFromHeap);
    SENTINEL_DEBUG_VAR("%d", (bool)patch_CreateTableFromAllocator);
    SENTINEL_DEBUG_VAR("%d", (bool)patch_CreateTableFromHeap);
}

void UpdateTable(std::string_view name, void* table)
{
    auto it = std::find_if(table_watches.begin(), table_watches.end(),
                           [name] (auto& x) { return x.first == name; });
    if (it != table_watches.cend()) {
        SENTINEL_DEBUG_MESSAGE("recording table \"%s\" at %p\n", name.data(), table);
        *(it->second) = table;
    } else {
        SENTINEL_DEBUG_MESSAGE("unrecorded table \"%s\" at %p\n", name.data(), table);
    }
}

} } // namespace reve::table

namespace {

void record_table(std::string_view name, void* table)
{
    reve::table::UpdateTable(name, table);
}

void delete_table(void* table)
{
    auto it = std::find_if(table_watches.begin(), table_watches.end(),
                           [table] (auto& x) { return *(x.second) == table; });
    if (it != table_watches.cend())
        reve::table::UpdateTable(it->first, nullptr);
}

} // namespace (anonymous)

