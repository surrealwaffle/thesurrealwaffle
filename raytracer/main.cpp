#include "main.h"

#include <d3d9.h>

#include <sentinel/window.hpp>
#include <sentutil/all.hpp>

#include "renderer.hpp"

namespace {

blamtracer::Renderer current_renderer;

blamtracer::RenderResult sample_renderer(LPDIRECT3DSURFACE9 pSurface);

bool Load()
{
    return sentutil::script::install_script_function<"sentinel_raytracer">(
        +[] (std::optional<bool> enable) -> bool
        {
            if (!enable)
                return current_renderer.is_installed();

            if (!enable.value()) {
                current_renderer.release();
            } else if (!current_renderer.is_installed()) {
                current_renderer = blamtracer::Renderer(sample_renderer);
                if (!current_renderer.try_install())
                    sentutil::console::cprintf("failed to install raytrace renderer");
            }

            return current_renderer.is_installed();
        }, "enables or disables the raytracing renderer"
    ) && sentutil::utility::manage_handle(sentinel_Events_DestroyEngineCallback(+[] { current_renderer.release(); }))
      && sentutil::utility::manage_handle(sentinel_video_ResetVideoDeviceCallback(+[] (LPDIRECT3DDEVICE9 device, D3DPRESENT_PARAMETERS*) { current_renderer.device_reset(device); }))
      && sentutil::utility::manage_handle(sentinel_video_AcquireVideoDeviceCallback(+current_renderer.device_acquire));

}

}

extern "C" DLL_EXPORT BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            return Load() ? TRUE : FALSE;
            break;

        case DLL_PROCESS_DETACH:
            // detach from process
            break;

        case DLL_THREAD_ATTACH:
            // attach to thread
            break;

        case DLL_THREAD_DETACH:
            // detach from thread
            break;
    }
    return TRUE; // succesful
}

namespace {

blamtracer::RenderResult sample_renderer(LPDIRECT3DSURFACE9 pSurface)
{
    using blamtracer::RenderResult;

    if (!pSurface)
        return RenderResult::fatal_error;

    D3DSURFACE_DESC desc = {};
    if (pSurface->GetDesc(&desc) != D3D_OK) {
        std::printf("could not get offscreen surface descriptor\n");
        return RenderResult::error;
    }

    if (desc.Format != D3DFMT_X8R8G8B8 && desc.Format != D3DFMT_A8R8G8B8) {
        std::printf("invalid surface format for the renderer");
        return RenderResult::fatal_error;
    }

    D3DLOCKED_RECT lockedRect = {};
    if (pSurface->LockRect(&lockedRect, NULL, 0) != D3D_OK) {
        std::printf("failed to lock offscreen surface\n");
        return RenderResult::error;
    }

    std::uint8_t* bytes = reinterpret_cast<std::uint8_t*>(lockedRect.pBits);

    const int stride = 4; // between pixels
    const auto row_margin = lockedRect.Pitch - desc.Width * stride;

    for (DWORD row = 0; row < desc.Height; ++row, bytes += row_margin) {
        for (DWORD column = 0; column < desc.Width; ++column, bytes += stride) {
            std::uint8_t& blue = bytes[0];
            std::uint8_t& green = bytes[1];
            std::uint8_t& red = bytes[2];

            red = 255;
            green = 30;
            blue = 255;
        }
    }

    pSurface->UnlockRect();
    return RenderResult::success;
}

}
