
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "engine.hpp"
#include "globals.hpp"

#include <sentinel/config.hpp>

#include <utility> // std::move

namespace {

sentinel::resource_list<reve::engine::camera_update_filter> camera_update_filters;
sentinel::resource_list<void(*)()> unload_game_callbacks;
sentinel::resource_list<void(*)()> destroy_engine_callbacks;

} // namespace (anonymous)

namespace reve { namespace engine {

update_netgame_flags_tproc proc_UpdateNetgameFlags = nullptr;
update_objects_tproc       proc_UpdateObjects      = nullptr;
update_tick_tproc          proc_UpdateTick         = nullptr;
update_camera_tproc        proc_UpdateCamera       = nullptr;
unload_game_instance_tproc proc_UnloadGameInstance = nullptr;
destroy_engine_tproc       proc_DestroyEngine      = nullptr;

extrapolate_local_unit_delta_tproc    proc_ExtrapolateLocalUnitDelta   = nullptr;
get_biped_update_position_flags_tproc proc_GetBipedUpdatePositionFlags = nullptr;
update_biped_position_tproc           proc_UpdateBipedPosition         = nullptr;

void hook_UpdateCamera(index_short local_index)
{
    proc_UpdateCamera(local_index);

    for (auto&& filter : camera_update_filters)
        filter(reve::globals::ptr_CameraGlobals);
}

void hook_UnloadGameInstance()
{
    for (const auto& cb : unload_game_callbacks)
        cb();

    proc_UnloadGameInstance();
}

void hook_DestroyEngine()
{
    for (const auto& cb : destroy_engine_callbacks)
        cb();

    proc_DestroyEngine();
}

sentinel_handle InstallCameraUpdateFilter(camera_update_filter&& filter)
{
    return camera_update_filters.push_back(std::move(filter));
}

sentinel_handle InstallUnloadGameCallback(void (*callback)())
{
    return unload_game_callbacks.push_back(callback);
}

sentinel_handle InstallDestroyEngineCallback(void (*callback)())
{
    return destroy_engine_callbacks.push_back(callback);
}

bool Init()
{
    return proc_UpdateNetgameFlags
        && proc_UpdateObjects
        && proc_UpdateTick
        && proc_UpdateCamera
        && proc_UnloadGameInstance
        && proc_DestroyEngine
        && proc_ExtrapolateLocalUnitDelta
        && proc_GetBipedUpdatePositionFlags
        && proc_UpdateBipedPosition;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", proc_UpdateNetgameFlags);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateObjects);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateTick);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateCamera);
    SENTINEL_DEBUG_VAR("%p", proc_UnloadGameInstance);
    SENTINEL_DEBUG_VAR("%p", proc_DestroyEngine);
    SENTINEL_DEBUG_VAR("%p", proc_ExtrapolateLocalUnitDelta);
    SENTINEL_DEBUG_VAR("%p", proc_GetBipedUpdatePositionFlags);
    SENTINEL_DEBUG_VAR("%p", proc_UpdateBipedPosition);
}

} } // namespace reve::engine
