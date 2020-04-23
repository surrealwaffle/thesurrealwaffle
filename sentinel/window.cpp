
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include <sentinel/window.hpp>

#include "reve/window.hpp"

SENTINEL_API
HWND
sentinel_window_GetWindow()
{
    return *reve::window::ptr_hWnd;
}

SENTINEL_API
sentinel::render_device_info*
sentinel_window_GetRenderDeviceInfo()
{
    return reve::window::ptr_RenderDeviceInfo;
}

SENTINEL_API
sentinel::cursor_info*
sentinel_window_GetCursorInfo()
{
    return reve::window::ptr_CursorInfo;
}
