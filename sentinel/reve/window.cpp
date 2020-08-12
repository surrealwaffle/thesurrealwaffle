
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "window.hpp"

#include <sentinel/config.hpp>

namespace reve { namespace window {

HWND*                  ptr_hWnd       = nullptr;
sentinel::cursor_info* ptr_CursorInfo = nullptr;

sentinel::VideoDevice* ptr_VideoDevice            = nullptr;
D3DPRESENT_PARAMETERS* ptr_PresentationParameters = nullptr;

renderer_begin_scene_tproc proc_RendererBeginScene = nullptr;
renderer_reset_device_tproc proc_RendererResetVideoDevice = nullptr;;

std::atomic<sentinel::CustomRendererFunction> custom_renderer = nullptr;

bool8 hook_RendererBeginScene()
{
    auto ready = proc_RendererBeginScene();

    auto renderer = custom_renderer.load();
    if (ready && renderer != nullptr && renderer()) {
        ptr_VideoDevice->requires_end_scene = false;
        return 0; // rendering handled
    }

    return ready;
}

detours::meta_patch patch_ResetVideoDevice;
bool8 tramp_ResetVideoDevice(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    // pre-reset callbacks

    patch_ResetVideoDevice.restore();
    const bool8 result = proc_RendererResetVideoDevice(pPresentationParameters);
    patch_ResetVideoDevice.repatch();

    if (result) {
        // post-acquired callbacks
    }

    return result;
}

bool Init()
{
    return ptr_hWnd
        && ptr_CursorInfo
        && ptr_VideoDevice
        && ptr_PresentationParameters
        && proc_RendererBeginScene;
}

void Debug()
{
    SENTINEL_DEBUG_VAR("%p", ptr_hWnd);
    SENTINEL_DEBUG_VAR("%p", ptr_CursorInfo);
    SENTINEL_DEBUG_VAR("%p", ptr_VideoDevice);
    SENTINEL_DEBUG_VAR("%p", ptr_PresentationParameters);
    SENTINEL_DEBUG_VAR("%p", proc_RendererBeginScene);
}

} } // namespace reve::window
