#include "renderer.hpp"

#include <cstdio>

#include <tuple>
#include <utility>

#include <sentinel/window.hpp>

namespace {

blamtracer::Renderer* current_renderer = nullptr;

}

namespace blamtracer {

Renderer::Renderer(rendering_function&& render)
    : surface(nullptr)
    , render(std::move(render))
    , install(nullptr)
{

}

Renderer::Renderer(Renderer&& other)
    : Renderer()
{
    *this = std::move(other);
}

Renderer& Renderer::operator=(Renderer&& other)
{
    release();
    std::tie(surface, render, install)
        = std::forward_as_tuple(std::move(other.surface),
                                std::move(other.render),
                                std::move(other.install));
    return *this;
}

void Renderer::release() noexcept
{
    if (is_installed())
        current_renderer = nullptr;

    [this] {
        if (surface) {
            LPDIRECT3DDEVICE9 device = sentinel_video_GetVideoDevice()->pDevice;
            if (device)
                surface->Release();
            surface = nullptr;
        }
    }();

    install.free_resource();
}

bool Renderer::try_install()
{
    const bool installed = [this] {
        if (!render) {
            std::printf("no rendering implementation\n");
            return false;
        }

        if (install) {
            std::printf("renderer already installed\n");
            return true;
        }

        auto invoke_current_renderer = +[] { return current_renderer != nullptr && current_renderer->try_render(); };
        if (!(install = sentinel_video_InstallCustomRenderer(invoke_current_renderer))) {
            std::printf("sentinel rejected :(\n");
            return false;
        }

        // create the offscreen surface
        LPDIRECT3DDEVICE9 device = sentinel_video_GetVideoDevice()->pDevice;
        const D3DPRESENT_PARAMETERS& parameters = *sentinel_video_GetPresentationParameters();

        if (!device || device->TestCooperativeLevel() != D3D_OK) {
            std::printf("no video device\n");
            return false; // no device
        }

        if (device->CreateOffscreenPlainSurface(parameters.BackBufferWidth,
                                                parameters.BackBufferHeight,
                                                D3DFMT_A8R8G8B8,
                                                D3DPOOL_DEFAULT,
                                                &surface,
                                                NULL) != D3D_OK) {
            std::printf("unable to create the offscreen buffer\n");
            return false; // could not create offscreen buffer
        }

        return true;
    }();

    if (installed) current_renderer = this;
    else           release();

    return installed;
}

bool Renderer::try_render()
{
    if (!is_installed()) {
        std::printf("could not render; not installed\n");
        return false;
    }

    auto render_result = render(surface);
    if (render_result == RenderResult::success) [this, &render_result] { // surface contains data, copy stuff to the backbuffer
        LPDIRECT3DDEVICE9 device = sentinel_video_GetVideoDevice()->pDevice;
        LPDIRECT3DSURFACE9 pBackBuffer = NULL;

        if (device->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer) != D3D_OK) {
            std::printf("could not acquire backbuffer\n");
            render_result = RenderResult::fatal_error;
            return;
        }

        device->EndScene(); // required for StretchRect
        if (device->StretchRect(surface, NULL, pBackBuffer, NULL, D3DTEXF_NONE) != D3D_OK) {
            std::printf("device->StretchRect() failed\n");
            render_result = RenderResult::fatal_error;
            device->BeginScene();
        }

        pBackBuffer->Release();
    }();

    if (render_result == RenderResult::fatal_error) {
        std::printf("fatal rendering error\n");
        render = nullptr;
        release();
    }

    return render_result == RenderResult::success;
}

}
