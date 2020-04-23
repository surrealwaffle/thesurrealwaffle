
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>

#include <sentinel/base.hpp>
#include <sentinel/events.hpp>

#include <sentutil/utility.hpp>

namespace sentutil { namespace events {

/** \brief Installs a callback that is invoked just after the map cache is loaded.
 *
 * The callback must be invocable with the name of the cache file.
 * The returned value of the invocation is immediately discarded.
 *
 * \param[in] cb The callback to install.
 *
 * \return `true` if and only if the callback was successfully installed.
 */
template<class Callback>
std::enable_if_t<
    std::is_invocable_v<Callback&, sentinel::h_ccstr>,
bool> install_load_map_cache_callback(Callback cb)
{
    sentinel::function<void(sentinel::h_ccstr)> callback(
        [cb] (auto cache_name) { (void)cb(cache_name); }
    );
    return utility::manage_handle(sentinel_Events_LoadMapCacheCallback(&callback));
}

/** \brief Installs a callback that is invoked just after the map is instantiated.
 *
 * The callback must be invocable without any parameters.
 * The returned value of the invocation is immediately discarded.
 *
 * \param[in] cb The callback to install.
 *
 * \return `true` if and only if the callback was successfully installed.
 */
template<class Callback>
std::enable_if_t<
    std::is_invocable_v<Callback&>,
bool> install_map_instantiation_callback(Callback cb)
{
    sentinel::function<void()> callback(
        [cb] { (void)cb(); }
    );
    return utility::manage_handle(sentinel_Events_InstantiateMapCallback(&callback));
}

} } // namespace sentutil::events
