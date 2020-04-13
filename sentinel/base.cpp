#include <sentinel/base.hpp>

#include "reve/table.hpp"
#include <sentinel/structures/player.hpp>
#include <sentinel/structures/table.hpp>

#include <algorithm> // std::find_if

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
int
sentinel_GetLocalPlayerNetworkIndex()
{
    auto* players = sentinel_Globals_tables.player_table;
    if (!players)
        return -1;

    auto is_desired_player
        = [] (const auto& player) { return player.local_index == 0; };

    auto player = std::find_if(players->cbegin(), players->cend(),
                               is_desired_player);

    if (player == players->cend())
        return -1;

    return player->network_index;
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
