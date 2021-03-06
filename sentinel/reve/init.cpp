
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "init.hpp"
#include "table.hpp"

#include <sentinel/config.hpp>
#include <sentinel/structures/globals.hpp>
#include <sentinel/tags/scenario.hpp>
#include <sentinel/fwd/globals_fwd.hpp>
#include <sentinel/fwd/table_fwd.hpp>

#include "../loader.hpp"

namespace {

using sentinel::resource_list;

resource_list<reve::init::load_map_cache_callback>  load_map_cache_callbacks;
resource_list<reve::init::instantiate_map_callback> instantiate_map_callbacks;

} // namespace (anonymous)

namespace reve { namespace init {

detours::meta_patch patch_InstantiateMap;

process_init_config_tproc  proc_ProcessInitConfig  = nullptr;
execute_init_config_tproc  proc_ExecuteInitConfig  = nullptr;
process_connect_args_tproc proc_ProcessConnectArgs = nullptr;
load_map_cache_tproc       proc_LoadMapCache       = nullptr;
instantiate_map_tproc      proc_InstantiateMap     = nullptr;
cleanup_game_tproc         proc_CleanupGame        = nullptr;

void hook_ProcessInitConfig()
{
    sentinel::impl_loader::LoadClientLibraries();
    return proc_ProcessInitConfig();
}

bool8 hook_ProcessConnectArgs()
{
    auto retval = proc_ProcessConnectArgs();
    sentinel::impl_loader::PerformSecondaryClientLoads();
    return retval;
}

bool8 hook_LoadMapCache(P_IN h_ccstr cache_name)
{
    auto success = proc_LoadMapCache(cache_name);

    if (success) {
        // -------------------------------------------------------
        // Some resources are set to an unacquired state until
        // the map is instantiated proper.
        table::UpdateTable("script node", nullptr);

        for (auto& cb : load_map_cache_callbacks)
            cb(cache_name);
    }

    return success;
}

void tramp_InstantiateMap()
{
    patch_InstantiateMap.restore();
    proc_InstantiateMap();
    patch_InstantiateMap.repatch();

    // -------------------------------------------------------
    // Some globals need to be updated after map instantiation
    table::UpdateTable("script node",
                       sentinel_Globals_globals.map_globals->scenario->script_node);

    // -----------------------------
    // Call client library callbacks
    for (auto& cb : instantiate_map_callbacks)
        cb();
}

void hook_CleanupGame()
{
    sentinel::impl_loader::UnloadClientLibraries();
    return proc_CleanupGame();
}

sentinel_handle InstallLoadMapCacheCallback(load_map_cache_callback&& callback)
{ return load_map_cache_callbacks.push_back(std::move(callback)); }

sentinel_handle InstallInstantiateMapCallback(instantiate_map_callback&& callback)
{ return instantiate_map_callbacks.push_back(std::move(callback)); }

bool Init()
{
    return proc_ProcessInitConfig
        && proc_ExecuteInitConfig
        && proc_ProcessConnectArgs
        && proc_LoadMapCache
        && proc_InstantiateMap && patch_InstantiateMap
        && proc_CleanupGame;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_ProcessInitConfig);
    SENTINEL_DEBUG_VAR("%p", proc_ExecuteInitConfig);
    SENTINEL_DEBUG_VAR("%p", proc_ProcessConnectArgs);
    SENTINEL_DEBUG_VAR("%p", proc_LoadMapCache);
    SENTINEL_DEBUG_VAR("%p", proc_InstantiateMap);
    SENTINEL_DEBUG_VAR("%d", (bool)patch_InstantiateMap);
    SENTINEL_DEBUG_VAR("%p", proc_CleanupGame);
}

} } // namespace reve::init
