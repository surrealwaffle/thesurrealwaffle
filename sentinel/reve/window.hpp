
//          Copyright surrealwaffle 2018 - 2019.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          https://www.boost.org/LICENSE_1_0.txt)

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

