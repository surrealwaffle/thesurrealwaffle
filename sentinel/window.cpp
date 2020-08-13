
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
sentinel::VideoDevice*
sentinel_video_GetVideoDevice()
{
    return reve::window::ptr_VideoDevice;
}

SENTINEL_API
sentinel::cursor_info*
sentinel_window_GetCursorInfo()
{
    return reve::window::ptr_CursorInfo;
}

SENTINEL_API
D3DPRESENT_PARAMETERS*
sentinel_video_GetPresentationParameters()
{
    return reve::window::ptr_PresentationParameters;
}

SENTINEL_API
sentinel_handle
sentinel_video_InstallCustomRenderer(sentinel::CustomRendererFunction renderer)
{
    sentinel::CustomRendererFunction expected_renderer = nullptr;
    using reve::window::custom_renderer;
    return custom_renderer.compare_exchange_strong(expected_renderer, renderer)
        ? sentinel::callback_handle([] (auto&&) { custom_renderer.store(nullptr); })
        : nullptr;
}

SENTINEL_API
sentinel_handle
sentinel_video_ResetVideoDeviceCallback(sentinel::ResetVideoDeviceCallback callback)
{
    return reve::window::add_device_reset_callback(callback);
}

SENTINEL_API
sentinel_handle
sentinel_video_AcquireVideoDeviceCallback(sentinel::AcquireVideoDeviceCallback callback)
{
    return reve::window::add_device_acquired_callback(callback);
}
