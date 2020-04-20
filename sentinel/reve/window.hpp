#pragma once

#include <wunduws.hpp>
#include <d3d9.h>

namespace sentinel {

struct render_device_info;

} // namespace sentinel

namespace reve { namespace window {

extern HWND*                         ptr_hWnd;
extern sentinel::render_device_info* ptr_RenderDeviceInfo;

bool Init();

void Debug();

} } // namespace reve::window

