
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

/** \file window.hpp
 * \brief Provisions for access and control to Halo's window and rendering contexts.
 *
 * This header is not included as part of `sentinel/all.hpp`, because it includes
 * both `windows.h` and `d3d9.h`.
 */

#pragma once

#include <cstddef>

#include <windows.h>
#include <d3d9.h>
#include <dinput.h>

#include <sentinel/config.hpp>
#include <sentinel/fundamental_types.hpp>
#include <sentinel/base.hpp>

namespace sentinel {

struct VideoDevice {
    void* heap1; // size of 64512 bytes, seems to be an array with stride 168 bytes, array of 384 elements
    void* heap2; // size of 5376 bytes, seems to be an array with stride 168 bytes, array of 32 elements
    int32 index_heap1; // indexes into array at heap1
    int32 index_heap2; // indexes into array at heap2
    void* heap3; // size of 768 bytes
    int16 unused0; // set but never read?
    void* rasterizer_data; // from the the game_globals tag, game_globals.rasterizer_data.pointer
    int32 KsUpdate_success; // 0 on failure, 1 on success, unchanged if KsUpdate was not called
    boolean is_fullscreen;
    boolean does_not_require_profile_reset; // 0 to indicate that the device needs to be reset on a profile switch, otherwise non-zero
    boolean requires_present; // Checked to skip a call to pDevice->Present(), but I don't know how this path is taken.
                              // Might require a fixed-function pipeline or vsync enabled.
    boolean requires_end_scene;
    boolean unknown1;
    LPDIRECT3DDEVICE9 pDevice;
    LPDIRECT3D9       pD3D;
}; static_assert(sizeof(VideoDevice) == 0x30);

// reorganize this sometime
struct InputGlobals {
	struct Unknown0 {
		h_long  milliseconds;
		h_short unknown;
	};

	struct BufferedKey {
		flags_byte modifiers; // ALT : CONTROL : SHIFT (LSB)
		h_char     character; // If the event was generated by `WM_CHAR` or `WM_SYSCHAR`: the lower byte of wParam
                              // If the event was generated by `WM_KEYDOWN` or `WM_SYSKEYDOWN`: `-1`
		h_short    keycode;   // internally translated, need work on this
	};

	struct MouseState {
        h_long horizontal; ///< Horizontal mouse movement (as viewed from above).
        h_long vertical;   ///< Vertical mouse movement (as viewed from above).
        h_long wheel;      ///< Mouse wheel, comes divided by granularity, if granularity is non-zero.

        /** \brief Indicates how long a mouse button has been held for.
         *
         * For every update of the mouse state, if a mouse button is down, then its
         * value this array increments up to `255`. If the button is not down, then
         * the value is set to `0`.
         *
         * Device state query rate is (generally) tied to framerate.
         * For high framerates, these button values reach the maximum quite quickly.
         */
        h_ubyte button_state[8];

        /** \brief Indicates that a mouse button has been released.
         *
         * Contains a non-zero value if the button has transitioned from a pressed
         * state to a released state.
         *
         * Effectively, this indicates that the corresponding value in #button_state
         * has gone from a non-zero value to `0`.
         */
        boolean button_changed[8];
	}; static_assert(sizeof(MouseState) == 0x1C);

	/** \brief Indicates that the enumerated devices are acquired with `true`,
	 *         or `false` if the devices are not acquired.
	 */
	boolean devices_acquired;

	/** \brief A switch that forces Halo to use various virtual input state buffers.
	 *
	 * This is never set `true` by Halo, but all the mechanisms that change input
	 * behavior are still implemented.
	 * For instance, if this value is `true`, then `enumerated_devices.direct_mouse_state`
	 * is ignored and mouse input is pulled from `enumerated_devices.virtual_mouse_state`.
	 *
	 * This was presumably implemented for Gearbox to have a consist device to test.
	 * This value is set to `false` when the input devices are unacquired.
	 */
	boolean use_virtual_input;

	LPDIRECTINPUT8 direct_input; ///< The `DirectInput` interface.

	Unknown0 unknown0[4]; // possibly last time of device update for each local player
	h_byte   unknown1[0x6D];
	h_byte   unknown2[0x6D];

	index_short buffered_key_read_index;
	h_short     buffered_keys_count;
	BufferedKey buffered_keys[0x40];

	struct {
	    /** \brief A device interface for the system keyboard device.
	     *
	     * The data format for this interface is `c_dfDIKeyboard`.
	     */
		LPDIRECTINPUTDEVICE direct_keyboard;

		/** \brief A device interface for the system mouse device.
		 *
		 * The data format for this interface is `c_dfDIMouse2`, for which
		 * `DIMOUSESTATE2` is appropriate for querying this device's buffered state.
		 */
		LPDIRECTINPUTDEVICE direct_mouse;
		h_ulong             direct_mouse_z_granularity; ///< Granularity for the z-axis (mouse wheel).
		MouseState          direct_mouse_state;  ///< The mouse state as *translated* from `direct_mouse->GetDeviceState`.
		MouseState          virtual_mouse_state; ///< Never written to by `Halo`, see #use_virtual_mouse.
		                                         ///< Presumably this used to be part of some harness
		                                         ///< for providing programmed control over the game.

		index_short         next_joystick; ///< The index of the next unoccupied joystick slot in #direct_joysticks.
		LPDIRECTINPUTDEVICE direct_joysticks[8]; ///< Interfaces for the registered gamepad/joystick devices.
		h_byte unknown5[0x240][8]; // joystick state
	} enumerated_devices;
}; static_assert(sizeof(InputGlobals) == 0x1470);

struct cursor_info {
    boolean unknown00;
    boolean unknown01;
    boolean unknown02;

    struct {
        h_long x;
        h_long y;
    } position;

    // quite a bit more goes here
};

using CustomRendererFunction     = bool(*)();
using ResetVideoDeviceCallback   = void(*)(LPDIRECT3DDEVICE9      device,
                                           D3DPRESENT_PARAMETERS* pPresentationParameters);
using AcquireVideoDeviceCallback = void(*)(LPDIRECT3DDEVICE9            device,
                                           const D3DPRESENT_PARAMETERS* pPresentationParameters);

static_assert(offsetof(VideoDevice, heap1) == 0x00);
static_assert(offsetof(VideoDevice, heap2) == 0x04);
static_assert(offsetof(VideoDevice, index_heap1) == 0x08);
static_assert(offsetof(VideoDevice, index_heap2) == 0x0C);
static_assert(offsetof(VideoDevice, heap3) == 0x10);
static_assert(offsetof(VideoDevice, rasterizer_data) == 0x18);
static_assert(offsetof(VideoDevice, KsUpdate_success) == 0x1C);
static_assert(offsetof(VideoDevice, is_fullscreen) == 0x20);
static_assert(offsetof(VideoDevice, requires_present) == 0x22);
static_assert(offsetof(VideoDevice, requires_end_scene) == 0x23);
static_assert(offsetof(VideoDevice, pDevice) == 0x28);
static_assert(offsetof(VideoDevice, pD3D) == 0x2C);

static_assert(offsetof(InputGlobals, initialized) == 0x000);
static_assert(offsetof(InputGlobals, direct_input) == 0x004);
static_assert(offsetof(InputGlobals, buffered_key_read_index) == 0x102);
static_assert(offsetof(InputGlobals, buffered_keys_count) == 0x104);
static_assert(offsetof(InputGlobals, buffered_keys) == 0x106);
static_assert(offsetof(InputGlobals, enumerated_devices.direct_keyboard) == 0x208);
static_assert(offsetof(InputGlobals, enumerated_devices.direct_mouse) == 0x20C);
static_assert(offsetof(InputGlobals, enumerated_devices.direct_mouse_z_granularity) == 0x210);
static_assert(offsetof(InputGlobals, enumerated_devices.direct_mouse_state) == 0x214);
static_assert(offsetof(InputGlobals, enumerated_devices.virtual_mouse_state) == 0x230);
static_assert(offsetof(InputGlobals, enumerated_devices.next_joystick) == 0x24C);
static_assert(offsetof(InputGlobals, enumerated_devices.direct_joysticks) == 0x250);
static_assert(offsetof(InputGlobals, enumerated_devices.unknown5) == 0x270);

}

extern "C" {

/** \brief Returns a handle to Halo's window,
 *         or `NULL` if there is no window created.
 */
SENTINEL_API
HWND
sentinel_window_GetWindow();

SENTINEL_API
sentinel::cursor_info*
sentinel_window_GetCursorInfo();


/** \brief Returns information and handles about the video device associated with
 *         the main window.
 */
SENTINEL_API
sentinel::VideoDevice*
sentinel_video_GetVideoDevice();

/** \brief Returns a pointer to the presentation parameters used for the video device.
 */
SENTINEL_API
D3DPRESENT_PARAMETERS*
sentinel_video_GetPresentationParameters();

/** \brief Installs \a renderer to be called to perform rendering.
 *
 * If installed, \a renderer is called after `BeginScene()` is invoked on the device.
 * The \a renderer must invoke `EndScene()` on a successful render.
 *
 * \a renderer should return `true` if it was successful in rendering.
 * A return value of `false` indicates to `sentinel` that the standard rendering path
 * should be used instead.
 */
SENTINEL_API
sentinel_handle
sentinel_video_InstallCustomRenderer(sentinel::CustomRendererFunction renderer);

/** \brief Installs a \a callback that is invoked before the video device is being
 *         reset (by `device->Reset()`).
 *
 * This callback may be invoked multiple times before the video device is acquired.
 */
SENTINEL_API
sentinel_handle
sentinel_video_ResetVideoDeviceCallback(sentinel::ResetVideoDeviceCallback callback);

/** \brief Installs a \a callback that is invoked after the video device is acquired
 *         from a call to `device->Reset()` that returns `D3D_OK`.
 *
 * Any of Halo's resources that are lost from the device reset are recreated by the
 * time the callback is invoked.
 */
SENTINEL_API
sentinel_handle
sentinel_video_AcquireVideoDeviceCallback(sentinel::AcquireVideoDeviceCallback callback);

} // extern "C"
