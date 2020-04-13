#include "memory.hpp"
#include "table.hpp" // table::RemoveRecordedTable

#include <sentinel/config.hpp>

namespace reve { namespace memory {

global_free_tproc  proc_GlobalFree        = nullptr;
global_free_tproc* import_proc_GlobalFree = nullptr;

void* hook_GlobalFree(void* hMem)
{
    table::SignalTableDestructing(hMem);
    return proc_GlobalFree(hMem);
}

bool Init()
{
    return proc_GlobalFree
        && import_proc_GlobalFree;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_GlobalFree);
    SENTINEL_DEBUG_VAR("%p", import_proc_GlobalFree);
}

} } // namespace reve::memory
