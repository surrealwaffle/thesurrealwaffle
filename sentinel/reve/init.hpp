
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <type_traits> // std::is_same

#include <detours/patch.hpp>
#include <sentinel/base.hpp>

#include "types.hpp"

namespace reve { namespace init {

using load_map_cache_callback  = sentinel::function<void(sentinel::h_ccstr name)>;
using instantiate_map_callback = sentinel::function<void()>;

/** \brief Loads and executes the init file.
 *
 * The default init file is `init.txt`.
 * If the `-exec <file>` switch is present, the supplied file is used instead.
 */
using process_init_config_tproc __attribute__((cdecl))
    = void(*)();

/** \brief Processes the command line arguments associated with connecting
 *         the client to a server on startup.
 *
 * Inspects the command-line arguments for the switch `-connect <ip:port>`.
 * If `-connect <ip:port>` is present, this procedure also searches for switches
 * `-name <name>` and `-password <password>`.
 *
 * \return `1` if the client should proceed to connect to a server, or
 *         `0` otherwise.
 */
using process_connect_args_tproc __attribute__((cdecl))
    = bool8(*)();

/** \brief Loads the map file into memory and does some basic preprocessing.
 *
 * Only the map and its tags are loaded into memory after this call.
 * Other globals, such as the objects table, are neither reset nor initialized
 * for the new map.
 *
 * \sa proc_LoadMapCache
 * \sa proc_InstantiateMap
 *
 * \param[in] cache_name The name of the map name (not the file path).
 *
 * \return Non-zero on success, or
 *         `0` on failure.
 */
using load_map_cache_tproc __attribute__((cdecl, regparm(1)))
    = bool8(*)(P_IN h_ccstr cache_name /*EAX*/);

using cleanup_game_tproc __attribute__((cdecl))
    = void(*)();

/** \brief Instantiates the current map's entities, such as objects.
 *
 * \sa proc_LoadMapCache
 * \sa proc_InstantiateMap
 */
using instantiate_map_tproc __attribute__((cdecl)) = void(*)();

extern detours::meta_patch patch_InstantiateMap;

extern process_init_config_tproc  proc_ProcessInitConfig;
extern process_connect_args_tproc proc_ProcessConnectArgs;
extern load_map_cache_tproc       proc_LoadMapCache;
extern instantiate_map_tproc      proc_InstantiateMap;
extern cleanup_game_tproc         proc_CleanupGame;

/** \brief Hook for \ref proc_ProcessInitConfig.
 *
 * Loads the libraries before the init file is processed, to allow for library
 * settings to be set via console commands.
 *
 * Note that a lot of globals during this call are unavailable.
 */
void hook_ProcessInitConfig() __attribute__((cdecl));

/** \brief Hook for \ref proc_ProcessConnectArgs.
 *
 * Performs a secondary load on client libraries. Unlike \ref hook_ProcessInitConfig,
 * this secondary load has available to it most of the globals.
 */
bool8 hook_ProcessConnectArgs() __attribute__((cdecl));

/** \brief Hook for \ref proc_LoadMapCache.
 *
 * Notifies client libraries of the map cache change after it has loaded into memory.
 */
bool8 hook_LoadMapCache(P_IN h_ccstr cache_name) __attribute__((cdecl, regparm(1)));

/** \brief Hook called when the game is being closed safely.
 *
 * Unloads client libraries.
 */
void hook_CleanupGame() __attribute__((cdecl));

/** \brief Trampoline for \ref proc_InstantiateMap.
 *
 * Notifies client libraries of the map instantiation after instantiation.
 */
void tramp_InstantiateMap() __attribute__((cdecl));

sentinel_handle InstallLoadMapCacheCallback(load_map_cache_callback&& callback);

sentinel_handle InstallInstantiateMapCallback(instantiate_map_callback&& callback);

bool Init();

void Debug();

static_assert(std::is_same_v<process_init_config_tproc,
                             decltype(&hook_ProcessInitConfig)>);
static_assert(std::is_same_v<process_connect_args_tproc,
                             decltype(&hook_ProcessConnectArgs)>);
static_assert(std::is_same_v<load_map_cache_tproc, decltype(&hook_LoadMapCache)>);
static_assert(std::is_same_v<instantiate_map_tproc, decltype(&tramp_InstantiateMap)>);
static_assert(std::is_same_v<cleanup_game_tproc, decltype(&hook_CleanupGame)>);

} } // namespace reve::init

