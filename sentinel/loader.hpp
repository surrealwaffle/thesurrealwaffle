
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "wunduws.hpp"

/** \file loader.hpp
 *
 * \brief Functions for loading and unloading application libraries.
 */

namespace sentinel { namespace impl_loader {

/** \brief Loads a client library into the application.
 *
 * Client libraries are found in the directory located by SENTINEL_APPLICATION_DIR.
 *
 * \param[in] lpModuleName The filename of the library.
 *
 * \return `true` if the library was loaded or is already loaded,
 *         otherwise `false` if the library cannot be found or fails to load.
 */
bool LoadClientLibrary(LPCTSTR lpModuleName);

/** \brief Unloads a client library loaded with \ref LoadClientLibrary.
 *
 * If the client library exports \ref SENTINEL_CLIENT_UNLOAD_PROC, it is interpreted
 * as a function pointer of type `void(*)()` and called.
 *
 * \param[in] lpModuleName The library name, as supplied to \ref LoadClientLibrary.
 *
 * \return `true` if the library was freed, otherwise `false`.
 */
bool UnloadClientLibrary(LPCTSTR lpModuleName);

/** \brief Loads all client libraries found in the client library directory.
 *
 * Internally invokes \ref LoadClientLibrary on each library.
 *
 * \sa LoadClientLibrary
 */
void LoadClientLibraries();

/** \brief Unloads all currently loaded client libraries.
 *
 * \sa UnloadClientLibrary
 */
void UnloadClientLibraries();

/** \brief Performs a secondary load on all client libraries.
 *
 * For each client library currently loaded, if the library exports
 * \ref SENTINEL_CLIENT_LOAD_PROC, this function invokes it as a `void(*)()`.
 */
void PerformSecondaryClientLoads();

} } // namespace sentinel::impl_loader
