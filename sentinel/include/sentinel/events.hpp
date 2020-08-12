
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/base.hpp>

#include <sentinel/structures/globals.hpp>

extern "C" {

/** \brief Installs \a callback to be called on when a map cache is loaded.
 *
 * The callback is called after the map cache has been loaded into memory, but before
 * the map is instantiated.
 *
 * To access the map tags, use \ref sentinel::globals_aggregate::map_globals.
 *
 * \sa sentinel_Events_InstantiateMapCallback
 *
 * \return A handle for the installed callback, or `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_Events_LoadMapCacheCallback(sentinel::function<void(sentinel::h_ccstr cache_name)>* callback);

/** \brief Installs \a callback to be called on when a map is being instantiated.
 *
 * The callback is called after the map has been instantiated.
 * This includes globals, tables, and map entities (such as scenery objects).
 *
 * \return A handle for the installed callback, or `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_Events_InstantiateMapCallback(sentinel::function<void()>* callback);

/** \brief Installs \a callback to be called when updating the camera.
 *
 * The callback is called after the builtin camera update.
 *
 * \return A handle for the installed callback, or `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_Engine_CameraUpdateCallback(sentinel::function<void(sentinel::camera_globals_type* camera)>* callback);

/** \brief Install a \a callback that is invoked when Halo leaves the main game loop
 *         and just before the resources of the current map instance are cleared.
 *
 * The game unload process consists mostly of cleaning the core state necessary to
 * maintain the state of the current game instance, e.g. the players table.
 * A number of allocations on the heap are freed, but the core remains allocated.
 *
 * \sa sentinel_Engine_DestroyEngineCallback
 */
SENTINEL_API
sentinel_handle
sentinel_Events_UnloadGameCallback(void (*callback)());

/** \brief Installs a \a callback that is invoked just before the client destroys
 *         engine resources.
 *
 * The engine destruction process unloads system resources from the client, including
 * the video device and related resources, the core, and the `keystone` library.
 */
SENTINEL_API
sentinel_handle
sentinel_Events_DestroyEngineCallback(void (*callback)());

} // extern "C""
