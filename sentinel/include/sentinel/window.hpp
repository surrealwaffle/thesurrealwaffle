
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** \file window.hpp
 * \brief Provides access to various handles, interfaces, and info relating to Halo's
 *        window and render context/device.
 *
 * This file is not included as part of `sentinel/all.hpp`, because it includes
 * both `windows.h` and `d3d9.h`, both of which may be undesirable.
 */

#pragma once

#include <cstddef>

#include <windows.h>
#include <d3d9.h>

#include <sentinel/config.hpp>
#include <sentinel/fundamental_types.hpp>

namespace sentinel {

struct render_device_info {
    void* heap1; // size of 64512 bytes, seems to be an array with stride 168 bytes, array of 384 elements
    void* heap2; // size of 5376 bytes, seems to be an array with stride 168 bytes, array of 32 elements
    int32 index_heap1; // indexes into array at heap1
    int32 index_heap2; // indexes into array at heap2
    void* heap3; // size of 768 bytes
    int16 unused0; // set but never read?
    void* rasterizer_data; // from the the game_globals tag, game_globals.rasterizer_data.pointer
    int32 KsUpdate_success; // 0 on failure, 1 on success, unchanged if KsUpdate was not called
    boolean is_fullscreen;
    boolean unknown0;
    boolean do_present;
    boolean do_end_scene;
    boolean unknown1;
    LPDIRECT3DDEVICE9 pDevice;
    LPDIRECT3D9       pD3D;
}; static_assert(sizeof(render_device_info) == 0x30);

static_assert(offsetof(render_device_info, heap1) == 0x00);
static_assert(offsetof(render_device_info, heap2) == 0x04);
static_assert(offsetof(render_device_info, index_heap1) == 0x08);
static_assert(offsetof(render_device_info, index_heap2) == 0x0C);
static_assert(offsetof(render_device_info, heap3) == 0x10);
static_assert(offsetof(render_device_info, rasterizer_data) == 0x18);
static_assert(offsetof(render_device_info, KsUpdate_success) == 0x1C);
static_assert(offsetof(render_device_info, is_fullscreen) == 0x20);
static_assert(offsetof(render_device_info, do_present) == 0x22);
static_assert(offsetof(render_device_info, do_end_scene) == 0x23);
static_assert(offsetof(render_device_info, pDevice) == 0x28);
static_assert(offsetof(render_device_info, pD3D) == 0x2C);

}

extern "C" {

/** \brief Returns a handle to Halo's window,
 *         or `NULL` if there is no window created.
 */
SENTINEL_API
HWND
sentinel_window_GetWindow();

/** \brief Returns the render device information.
 */
SENTINEL_API
sentinel::render_device_info*
sentinel_window_GetRenderDeviceInfo();

} // extern "C"
