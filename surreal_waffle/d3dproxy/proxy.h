#ifndef PROXY_H
#define PROXY_H

/*
	proxy.h:
		Utility provisions for the setup of this program as a proxy DLL.
*/

#include "stdafx.h"

#include "proxIDirect3D9.h"
#include "proxIDirect3D9Device.h"

/******************
 * Globals
 ******************/

// A handle to an instance of this proxy DLL.
extern HINSTANCE	g_hProxy;

// A handle to the loaded d3d9.dll from the system directory.
extern HMODULE		g_hBackEnd;

// A pointer to the wrapping Direct3D9 interface.
extern proxIDirect3D9		*g_lpIDirect3D9;

// A pointer to the wrapping Direct3D9Device interface.
extern proxIDirect3DDevice9	*g_lpIDirect3DDevice9;

/******************
 * Prototypes
 ******************/

// The entry point of this proxy library.
// This function calls ProxyAttach( ) and ProxyDetach( ) when this DLL is attached and detached, respectively.
BOOL APIENTRY DllMain( 
	HINSTANCE	hInstance,
	DWORD		fdwReason,
	LPVOID		lpvReserved 
	);

// Performs initialization of this proxy DLL.
BOOL ProxyAttach( HINSTANCE const hInstance );

// Performs cleanup of this proxy DLL.
BOOL ProxyDetach( HINSTANCE const hInstance );

#endif // PROXY_H