
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits>

#include "types.hpp"

namespace reve { namespace memory {

using global_free_tproc __attribute__((stdcall)) = void*(*)(void*);

/** \brief A pointer to the `GlobalFree` function as imported.
 */
extern global_free_tproc  proc_GlobalFree;

/** \brief A pointer to the `GlobalFree` import (that is, a pointer to a pointer).
 */
extern global_free_tproc* import_proc_GlobalFree;

/** \brief Replacement procedure for \ref import_proc_GlobalFree.
 *
 * Checks among various modules to see if \a hMem is referenced.
 *
 * \sa SignalTableDestructing
 */
void* hook_GlobalFree(void* hMem) __attribute__((stdcall));

bool Init();

void Debug();

static_assert(std::is_same_v<global_free_tproc, decltype(&hook_GlobalFree)>);

} } // namespace reve::memory

