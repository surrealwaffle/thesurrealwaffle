#include "main.h"

#include <cmath>

#include <atomic>
#include <iostream>

#include <sentinel/window.hpp>
#include <sentutil/all.hpp>

#include "renderer.hpp"
#include "thread_pool.hpp"

namespace {

blamtracer::thread_pool  thread_pool(6);
float                    fovy         = 0.94247779607694f; // 54 degrees
float                    ray_distance = 4.0f;
blamtracer::Renderer     current_renderer;
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
    ) && sentutil::script::install_script_function<"sentinel_raytracer_distance">(
        +[] (std::optional<float> value) -> float
        {
            if (value) ray_distance = std::max(value.value(), 0.0f);
            return ray_distance;
        }, "changes the maximum distance at which rays may be cast"
    ) && sentutil::utility::manage_handle(sentinel_Events_UnloadGameCallback(
        +[]
        {
            current_renderer.release();
            thread_pool.shutdown();
        }))
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

struct projective_settings {
    projective_settings(float fovy, float width, float height)
        : aspect(width / height)
        , fovy(fovy)
        , fovh(2.0f * std::atan(std::tan(fovy / 2.0f) * aspect))
        , tan_half_fovy(std::tan(fovy / 2.0f))
        , tan_half_fovh(tan_half_fovy * aspect)
        , width(width)
        , inverse_width(1 / width)
        , height(height)
        , inverse_height(1 / height) { }

    float aspect;
    float fovy;
    float fovh;

    float tan_half_fovy;
    float tan_half_fovh;

    float width;
    float inverse_width;
    float height;
    float inverse_height;
};

struct alignas(std::uint32_t) color {
    std::uint8_t blue;
    std::uint8_t green;
    std::uint8_t red;
    std::uint8_t alpha;
};

color get_color(std::optional<sentinel::identity<sentinel::object>> source_object,
                const sentinel::affine_matrix3d& camera,
                const projective_settings& settings,
                float x, float y)
{
    constexpr float near_plane = 1.0f / 16.0f;

    // x, y are in screen coordinates
    // need to convert them to NDC, with z = 0 implicit
    x = (2.0f * x - settings.width) * settings.inverse_width;
    y = -(2.0f * y - settings.height) * settings.inverse_height;

    // now NDC to camera coordinates
    // this is not as complex as it sounds...
    x *= near_plane * settings.tan_half_fovh;
    y *= near_plane * settings.tan_half_fovy;

    constexpr color no_hit = {0, 0, 0, 0};
    constexpr color hit_water = {226, 142, 86, 255};
    constexpr color hit_environment = {93, 153, 220, 255};
    constexpr color hit_object = {86, 226, 118, 255};
    constexpr color hit_unknown = {255, 40, 255, 255};

    const sentinel::real3d ray_direction = normalized(camera.ortho_transform * sentinel::real3d{near_plane, -x, y});

    if (auto result = sentutil::raycast::cast_projectile_ray(camera.translation, ray_distance * ray_direction, source_object.value_or(sentinel::invalid_identity))) {
        switch (result.value().hit_type) {
        case 0: return hit_water;
        case 2: return hit_environment;
        case 3: return hit_object;
        default: return hit_unknown;
        }
    }

    return no_hit;
}

// Annoying but necessary to avoid allocations for each task created.
namespace job_info {

std::uint8_t*                        bytes = nullptr;
sentinel::identity<sentinel::object> source_object = sentinel::invalid_identity;
sentinel::affine_matrix3d            camera;
projective_settings                  settings = {fovy, 800, 600};
std::atomic<long>                    row   = 0L;
long                                 width = 0L;
DWORD                                pitch = 0L;
constexpr int                        stride = 4;

void process_next_row()
{
    long job_col = 0L;
    const long job_row = row.fetch_add(1L, std::memory_order_acquire);
    std::uint8_t* job_bytes = bytes + job_row * pitch + job_col * stride;

    float screen_x = 0.5f;
    const float screen_y = 0.5f + job_row;
    for (long job_col = 0L, end = width; job_col < end; ++job_col, job_bytes += stride, screen_x += 1.0f) {
        std::uint8_t& blue = job_bytes[0];
        std::uint8_t& green = job_bytes[1];
        std::uint8_t& red = job_bytes[2];
        std::uint8_t& alpha = job_bytes[3];

        color&& pixel = get_color(source_object, camera, settings, screen_x, screen_y);

        blue = pixel.blue;
        green = pixel.green;
        red = pixel.red;
        alpha = pixel.alpha;
    }
}

void process_image(std::uint8_t* pImage, const D3DSURFACE_DESC& desc, DWORD imagePitch)
{
    const auto& camera_globals = *sentutil::globals::camera_globals;

    bytes = pImage;
    source_object = sentinel_GetLocalPlayerUnit();
    camera = sentinel::affine_matrix3d{
        1.0f,
        {
            camera_globals.forward,
            cross(camera_globals.up, camera_globals.forward),
            camera_globals.up
        },
        camera_globals.position
    };
    settings = projective_settings(fovy, desc.Width, desc.Height);
    row.store(0L, std::memory_order_release);
    width = desc.Width;
    pitch = imagePitch;

    thread_pool.push_tasks(desc.Height, [] (...) { return process_next_row; }).join();
    //for (auto rows_left = desc.Height; rows_left; --rows_left)
    //  process_next_row();
}

}

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

    job_info::process_image(bytes, desc, lockedRect.Pitch);

    pSurface->UnlockRect();
    return RenderResult::success;
}

}
