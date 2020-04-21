
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "window.hpp"

#include <sentinel/config.hpp>

namespace reve { namespace window {

HWND*                         ptr_hWnd             = nullptr;
sentinel::render_device_info* ptr_RenderDeviceInfo = nullptr;

bool Init()
{
    return ptr_hWnd
        && ptr_RenderDeviceInfo;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", ptr_hWnd);
    SENTINEL_DEBUG_VAR("%p", ptr_RenderDeviceInfo);
}

} } // namespace reve::window
