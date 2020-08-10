
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <atomic>
#include <memory>
#include <type_traits>

#include "sentinel/window.hpp"
#include "types.hpp"

namespace reve { namespace window {

/** \brief Called by Halo's rendering system to acquire the device (if lost) and
 *         prepare the scene for rendering.
 *
 * If a non-zero value is returned, then `device->BeginScene()` has been called.
 *
 * \return `0` if the device is not ready, otherwise non-zero to render the scene.
 */
using renderer_begin_scene_tproc __attribute__((cdecl))
    = bool8(*)();

extern HWND*                  ptr_hWnd;
extern sentinel::cursor_info* ptr_CursorInfo;

extern sentinel::VideoDevice* ptr_VideoDevice;
extern D3DPRESENT_PARAMETERS* ptr_PresentationParameters;

extern renderer_begin_scene_tproc proc_RendererBeginScene;

extern std::atomic<sentinel::CustomRendererFunction> custom_renderer;

/** \brief The replacement procedure for #proc_RendererBeginScene.
 *
 * Prepares the device for rendering, by calling #proc_RendererBeginScene.
 * If unsuccessful, this hook simply returns to Halo.
 *
 * Otherwise, if the device is ready and a custom renderer is installed, then the
 * custom renderer is invoked and the value returned from this hook is modified to
 * prevent Halo from following the standard rendering path.
 *
 * If the device is ready but no custom renderer is available, then the standard
 * rendering path is used.
 */
bool8 hook_RendererBeginScene();

bool Init();

void Debug();

static_assert(std::is_same_v<renderer_begin_scene_tproc,
                             decltype(&hook_RendererBeginScene)>);

} } // namespace reve::window

