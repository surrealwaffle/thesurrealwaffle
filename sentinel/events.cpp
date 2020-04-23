
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/events.hpp>
#include "reve/table.hpp"
#include "reve/init.hpp"

#include <utility> // std::move

using sentinel::function;
using sentinel::h_ccstr;

SENTINEL_API
sentinel_handle
sentinel_Events_LoadMapCacheCallback(function<void(h_ccstr cache_name)>* callback)
{
    return callback ? reve::init::InstallLoadMapCacheCallback(std::move(*callback))
                    : nullptr;
}

SENTINEL_API
sentinel_handle
sentinel_Events_InstantiateMapCallback(function<void()>* callback)
{
    return callback ? reve::init::InstallInstantiateMapCallback(std::move(*callback))
                    : nullptr;
}
