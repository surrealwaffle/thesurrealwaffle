
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "types.hpp"

#include <sentinel/base.hpp>

#include <type_traits>

namespace sentinel {
    struct camera_globals_type;
}

namespace reve { namespace engine {

/** \brief Updates the netgame flags by a single tick, including teleporters.
 *
 * \sa proc_UpdateNetgameFlags
 */
using update_netgame_flags_tproc __attribute__((cdecl)) = void(*)();

/** \brief Updates all objects by a single tick.
 *
 * \sa proc_UpdateObjects
 */
using update_objects_tproc __attribute__((cdecl)) = void(*)();

/** \brief Updates the game by a single tick.
 *
 * Includes #proc_UpdateNetgameFlags and #proc_UpdateObjects, among others.
 * \a ticks_remaining is the number of ticks remaining in the cumulative update.
 *
 * A call to this function should appropriately increment the tiok counters in the
 * game time globals structure, to remaing consistent with Halo's behaviour.
 */
using update_tick_tproc __attribute__((cdecl))
    = void(*)(P_IN ticks_long ticks_remaining /* STACK */);

/** \brief Calculates the camera position for the current frame.
 */
using update_camera_tproc __attribute__((cdecl, regparm(1)))
    = void(*)(P_IN index_short local_index /*EAX*/);

using camera_update_filter = sentinel::function<void(sentinel::camera_globals_type* camera)>;

/** \brief Extrapolates the change in position for local player's unit.
 *
 * This function is called on to calculate the camera position between ticks.
 *
 * \return `true` if the output structures contain data, otherwise `false`.
 */
using extrapolate_local_unit_delta_tproc __attribute__((cdecl))
    = bool8(*)(P_OUT real* deltas   /*[3], STACK*/,
               P_OUT real* unknown0 /*[3], STACK*/,
               P_OUT real* unknown1 /*[3], STACK*/,
               real seconds);

/** \brief Retrieves biped control flags for use in \ref proc_UpdateBipedPosition.
 */
using get_biped_update_position_flags_tproc __attribute__((cdecl, regparm(1)))
    = void(*)(P_IN  identity_raw identity /*EAX*/,
              P_OUT flags_short* pFlags   /*stack*/);

/** \brief Updates the biped of the given identity.
 *
 * \a control_flags should be initialized as described in \ref get_biped_update_position_flags_tproc.
 */
using update_biped_position_tproc __attribute__((cdecl))
    = void(*)(P_IN identity_raw identity      /*stack*/,
              P_IN flags_short* control_flags /*stack*/);

extern update_netgame_flags_tproc proc_UpdateNetgameFlags;
extern update_objects_tproc       proc_UpdateObjects;
extern update_tick_tproc          proc_UpdateTick;
extern update_camera_tproc        proc_UpdateCamera;

extern extrapolate_local_unit_delta_tproc    proc_ExtrapolateLocalUnitDelta;
extern get_biped_update_position_flags_tproc proc_GetBipedUpdatePositionFlags;
extern update_biped_position_tproc           proc_UpdateBipedPosition;

void hook_UpdateCamera(index_short local_index) __attribute__((cdecl, regparm(1)));

sentinel_handle InstallPreCumulativeTickCallback();
sentinel_handle InstallPostCumulativeTickCallback();
sentinel_handle InstallPreTickCallback();
sentinel_handle InstallPostTickCallback();

sentinel_handle InstallCameraUpdateFilter(camera_update_filter&& filter);

bool Init();

void Debug();

static_assert(std::is_same_v<update_camera_tproc, decltype(&hook_UpdateCamera)>);

} } // namespace reve::engine
