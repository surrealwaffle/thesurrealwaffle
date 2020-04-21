
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <string_view>

#include <detours/patch.hpp>
#include "types.hpp"

namespace reve { namespace table {

/** \brief Removes an element by its \a identity from the supplied \a table.
 *
 * Attempts to write `0` to the word at address `0`0 if any of the following are true:
 *  * the low word in \a identity is negative;
 *  * the low word in \a identity exceeds the index of table's last active element;
 *  * the high word in \a identity or the first word of the referred element are `0`;
 *  * the two just aforementioned words do not match.
 *
 * The low word of \a identity is the element index and its high word is salt.
 * The salt is stored as the first word of the element and is `0` if the element is
 * not allocated.
 */
using remove_table_element_tproc __attribute__((cdecl, regparm(2)))
    = void(*)(P_IN void*         table    /*EAX*/,
              P_IN identity_raw  identity /*EDX*/);

using create_table_tproc
    = exotic_function<void*(P_IN int16   datum_size  /*[EBX]*/,
                            P_IN h_ccstr szName      /*[stack]*/,
                            P_IN int16   datum_count /*[stack]*/)>;

extern detours::meta_patch patch_CreateTableFromAllocator;
extern detours::meta_patch patch_CreateTableFromHeap;

// this function is simple to reverse engineer
// consider implementing it plainly
extern remove_table_element_tproc proc_RemoveTableElement;
extern create_table_tproc         proc_CreateTableFromAllocator; // exotic
extern create_table_tproc         proc_CreateTableFromHeap;      // exotic


extern "C"
void*
sentinel_handler_CreateTable(int32 to_heap,
                             int16 datum_size,
                             h_ccstr szName,
                             int16 datum_count)
                             __attribute__((cdecl));

void*
tramp_CreateTableFromAllocator(/*P_IN int16       datum_size  [EBX],
                                 P_IN h_ccstr     szName      [stack],
                                 P_IN int16       datum_count [stack]*/)
                               __attribute__((cdecl, naked));

void*
tramp_CreateTableFromHeap(/*P_IN int16       datum_size  [EBX],
                            P_IN h_ccstr     szName      [stack],
                            P_IN int16       datum_count [stack]*/)
                          __attribute__((cdecl, naked));

/** \brief Call this function to signal that \a table is being destroyed.
 *
 * If \a table is not a recorded table, then nothing is done.
 */
void SignalTableDestructing(void* table);

bool Init();

void Debug();

void UpdateTable(std::string_view name, void* table);

} } // namespace reve::table

