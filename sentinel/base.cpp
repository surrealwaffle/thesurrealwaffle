
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/base.hpp>

#include "reve/init.hpp"
#include "reve/table.hpp"
#include <sentinel/structures/player.hpp>
#include <sentinel/structures/table.hpp>
#include <sentinel/console.hpp>

#include <algorithm> // std::find_if
#include <fstream>   // std::ifstream

struct sentinel_handle_type {
    using resource_type = void*;
    using release_proc  = void(*)(sentinel_handle, void*);

    resource_type resource;
    release_proc  release;
};

SENTINEL_API
sentinel_handle sentinel_MakeHandle(void* resource,
                                    void (*release)(sentinel_handle, void*))
{
    try {
        return new sentinel_handle_type{resource, release};
    } catch (...) {
        return nullptr;
    }
}

SENTINEL_API
void sentinel_FreeHandle(sentinel_handle handle) {
    if (handle) {
        if (handle->release)
            handle->release(handle, handle->resource);
        handle->resource  = nullptr;
        handle->release = nullptr;
        delete handle;
    }
}

SENTINEL_API
sentinel::identity<sentinel::player>
sentinel_GetLocalPlayer()
{
    auto* players = sentinel_Globals_tables.player_table;
    if (!players)
        return sentinel::invalid_identity;

    auto is_local = [] (const auto& player) { return player.is_local(); };
    auto player = std::find_if(players->cbegin(), players->cend(), is_local);

    if (player == players->cend())
        return sentinel::invalid_identity;

    return sentinel::identity<sentinel::player>::piecewise(player.base() - players->cbegin().base(), player->salt);
}

SENTINEL_API
sentinel::identity<sentinel::unit>
sentinel_GetLocalPlayerUnit()
{
    auto player = sentinel_GetLocalPlayer();
    return player ? player->unit : sentinel::invalid_identity;
}

SENTINEL_API
int
sentinel_GetLocalPlayerNetworkIndex()
{
    auto player = sentinel_GetLocalPlayer();
    return player ? player->network_index : -1;
}

SENTINEL_API
bool
sentinel_ExecuteConfigFile(const char* lpszFile)
{
    if (!sentinel_Globals_tables.script_node_table ||
        !sentinel_Globals_tables.script_thread_table ||
        lpszFile == nullptr)
        return false;

    return reve::init::proc_ExecuteInitConfig(lpszFile);
}

#ifdef SENTINEL_BUILD_DLL

namespace sentinel {

sentinel_handle callback_handle(const std::function<void(sentinel_handle)>& on_free)
{
    using function_type = std::function<void(sentinel_handle)>;
    auto free = [] (sentinel_handle handle, void* resource) {
        resource = __builtin_assume_aligned(resource, alignof(function_type));
        function_type* function_ptr = reinterpret_cast<function_type*>(resource);
        (*function_ptr)(handle);
        delete function_ptr;
    };

    return sentinel_MakeHandle(new function_type(on_free), +free);
}

sentinel_handle callback_handle(std::function<void(sentinel_handle)>&& on_free)
{
    using function_type = std::function<void(sentinel_handle)>;
    auto free = [] (sentinel_handle handle, void* resource) {
        resource = __builtin_assume_aligned(resource, alignof(function_type));
        function_type* function_ptr = reinterpret_cast<function_type*>(resource);
        (*function_ptr)(handle);
        delete function_ptr;
    };

    return sentinel_MakeHandle(new function_type(std::move(on_free)), +free);
}

} // namespace sentinel

#endif // SENTINEL_BUILD_DLL
