
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <sentinel/config.hpp>
#include <sentinel/base.hpp>

extern "C" {

/** \brief Installs \a callback to be called on when a map cache is loaded.
 *
 * The filter is called after the map cache has been loaded into memory, but before
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
sentinel_Events_LoadMapCacheCallback(
    sentinel::function<void(sentinel::h_ccstr cache_name)>* callback);

/** \brief Installs \a filter to be called on when a map is being instantiated.
 *
 * The filter is called after the map has been instantiated.
 * This includes globals, tables, and map entities (such as scenery objects).
 *
 * \return A handle for the installed callback, or `nullptr` on failure.
 */
SENTINEL_API
sentinel_handle
sentinel_Events_InstantiateMapCallback(sentinel::function<void()>* callback);

} // extern "C""
