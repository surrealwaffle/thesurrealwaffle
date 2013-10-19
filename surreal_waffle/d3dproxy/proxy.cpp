#include "proxy.h"

#include <string>

// initialize globals
HINSTANCE		g_hProxy		= NULL;
HMODULE			g_hBackEnd		= NULL;
proxIDirect3D9 *g_lpIDirect3D9	= NULL;
proxIDirect3DDevice9 *g_lpIDirect3DDevice9 = NULL;

#define LOAD_TARGET_BUFLEN	512
#define LOAD_TARGET_TARGET	"d3d9.dll"

// Utility code:
// Returns a handle to the target LOAD_TARGET_TARGET loaded in the system's directory.
// If this function fails, it returns NULL.
HMODULE LoadTarget( void ) {
	TCHAR buf[ LOAD_TARGET_BUFLEN ];

	// get the system directory
	UINT const uiStrLen = GetSystemDirectory( buf, sizeof( buf ) / sizeof( TCHAR ) );

	// ensure system dir is in buf
	if ( uiStrLen == NULL )
		return NULL;

	// create the filepath used to load the library
	std::string const strFile = std::string( buf ) + "\\" LOAD_TARGET_TARGET;

	return LoadLibrary( strFile.c_str( ) );
}

// Performs initialization of this proxy DLL.
// Returns TRUE if initialization was successful.
// Else, returns FALSE.
BOOL ProxyAttach( HINSTANCE const hInstance ) {
	// initialize the global variables declared in the proxy header
	::g_hProxy = hInstance;

	// load the original d3d9 library
	HMODULE const hRealLib = LoadTarget( );

	::g_hBackEnd = hRealLib;

	return hRealLib != NULL ? TRUE : FALSE;
}

// Performs cleanup of this proxy DLL.
// Returns TRUE if cleanup of used resources was successful.
// Else, returns FALSE.
BOOL ProxyDetach( HINSTANCE const hInstance ) {
	return ::g_hBackEnd != NULL && FreeLibrary( ::g_hBackEnd ) != NULL ? TRUE : FALSE;
}

// The entry point of this proxy library.
// This function calls OnProxyAttach( ) and OnProxyDetach( ) when this DLL is attached and detached, respectively.
BOOL APIENTRY DllMain( HINSTANCE hInstance, DWORD fdwReason, LPVOID ) {
	// doesn't actually mean anything except when fdwReason == DLL_PROCESS_ATTACH
	// see MSDN: http://msdn.microsoft.com/en-us/library/windows/desktop/ms682583%28v=vs.85%29.aspx
	BOOL resVal = TRUE;

	switch ( fdwReason ) {
	case DLL_PROCESS_ATTACH:
		resVal = ProxyAttach( hInstance );
		break;
	case DLL_PROCESS_DETACH:
		resVal = ProxyDetach( hInstance );
		break;
	// dont deal with the other cases
	}

	return resVal;
}