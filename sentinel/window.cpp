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
