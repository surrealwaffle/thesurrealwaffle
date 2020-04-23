
//          Copyright surrealwaffle 2018 - 2020.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

#include "hook.hpp"

#include <iostream>
#include <iterator>
#include <sentinel/window.hpp>
#include <sentutil/all.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx9.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {

bool imgui_initialized = false;
bool show_imgui        = true;

HWND              hWnd          = NULL;
WNDPROC           lpPrevWndFunc = NULL;
LPDIRECT3DDEVICE9 pDevice       = NULL;

LRESULT CALLBACK hkWindowProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam);

HRESULT STDMETHODCALLTYPE hkEndScene(
    IDirect3DDevice9* pDevice);

HRESULT STDMETHODCALLTYPE hkReset(
    IDirect3DDevice9*      pDevice,
    D3DPRESENT_PARAMETERS* pPresentationParameters);

void hkControlsFilter(sentinel::digital_controls_state& digital,
                      sentinel::analog_controls_state&  analog,
                      float seconds,
                      long ticks);

void**                device_vtable     = nullptr;
decltype(&hkEndScene) original_EndScene = nullptr;
decltype(&hkReset)    original_Reset    = nullptr;

} // namespace (anonymous)

namespace hook {

#define HOOK_ENDSCENE_VTABLE_INDEX 42
#define HOOK_RESET_VTABLE_INDEX    16

bool init()
{
    hWnd    = sentinel_window_GetWindow();
    pDevice = sentinel_window_GetRenderDeviceInfo()->pDevice;

    if (hWnd == NULL) {
        std::cout << "window handle is NULL" << std::endl;
        return false;
    } else if (pDevice == NULL) {
        std::cout << "rendering device was not located" << std::endl;
        return false;
    }

    lpPrevWndFunc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, LONG_PTR(&hkWindowProc)));

    device_vtable = *reinterpret_cast<void***>(pDevice);
    original_EndScene = reinterpret_cast<decltype(original_EndScene)>(device_vtable[HOOK_ENDSCENE_VTABLE_INDEX]);
    original_Reset    = reinterpret_cast<decltype(original_Reset)>(device_vtable[HOOK_RESET_VTABLE_INDEX]);
    device_vtable[HOOK_ENDSCENE_VTABLE_INDEX] = (void*)&hkEndScene;
    device_vtable[HOOK_RESET_VTABLE_INDEX]    = (void*)&hkReset;

    return sentutil::controls::install_controls_filter(hkControlsFilter);
}

void shutdown()
{
    std::cout << "hook::shutdown()" << std::endl;
    if (lpPrevWndFunc) {
        (void)SetWindowLongPtr(hWnd, GWLP_WNDPROC, LONG_PTR(lpPrevWndFunc));

        hWnd          = NULL;
        lpPrevWndFunc = nullptr;
    }

    // vtable cleanup must be done BEFORE cleaning up ImGui
    if (device_vtable) {
        device_vtable[HOOK_ENDSCENE_VTABLE_INDEX] = (void*)original_EndScene;
        device_vtable[HOOK_RESET_VTABLE_INDEX]    = (void*)original_Reset;

        pDevice = NULL;
        device_vtable     = nullptr;
        original_EndScene = nullptr;
        original_Reset    = nullptr;
    }

    if (imgui_initialized) {
        ImGui_ImplDX9_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();

        imgui_initialized = false;
    }
}

#undef HOOK_ENDSCENE_VTABLE_INDEX
#undef HOOK_RESET_VTABLE_INDEX

} // namespace hook

namespace {

LRESULT CALLBACK hkWindowProc(
    HWND   hWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    if (imgui_initialized && show_imgui) {
        LRESULT result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
        if (result)
            return result;
    }

    return CallWindowProc(lpPrevWndFunc, hWnd, uMsg, wParam, lParam);
}

HRESULT STDMETHODCALLTYPE hkEndScene(
    IDirect3DDevice9* pDevice)
{
    if (!imgui_initialized) {
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(hWnd);
        ImGui_ImplDX9_Init(pDevice);
        imgui_initialized = true;

        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;
    }

    if (imgui_initialized && show_imgui) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            constexpr int vkcodes[] = {
                VK_LBUTTON,
                VK_RBUTTON,
                VK_MBUTTON,
                VK_XBUTTON1,
                VK_XBUTTON2
            };

            for (auto it = std::begin(vkcodes); it != std::end(vkcodes); ++it) {
                io.MouseDown[it - std::begin(vkcodes)] = ::GetAsyncKeyState(*it) < 0;
            }
        }
        io.MouseDrawCursor = true;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableSetMousePos;

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(&show_imgui);
        ImGui::EndFrame();

        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return original_EndScene(pDevice);
}

HRESULT STDMETHODCALLTYPE hkReset(
    IDirect3DDevice9*      pDevice,
    D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hResult = original_Reset(pDevice, pPresentationParameters);
    ImGui_ImplDX9_CreateDeviceObjects();

    return hResult;
}

void hkControlsFilter(sentinel::digital_controls_state& digital,
                      sentinel::analog_controls_state&  analog,
                      float,
                      long)
{
    if (!imgui_initialized || !show_imgui)
        return;

    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        digital = {/*ZERO INITIALIZED*/};

    if (io.WantCaptureMouse)
        analog = {/*ZERO INITIALIZED*/};
}

} // namespace (anonymous)
