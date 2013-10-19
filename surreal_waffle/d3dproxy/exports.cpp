#include "exports.h"
#include "proxy.h"

void	WINAPI	Direct3DShaderValidatorCreate9( void ) {
	typedef void (WINAPI *T)(void);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "Direct3DShaderValidatorCreate9" );
	((T)proc)();
}

int		WINAPI	D3DPERF_BeginEvent( D3DCOLOR col, LPCWSTR wszName ) {
	typedef int (WINAPI *T)(D3DCOLOR, LPCWSTR);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_BeginEvent" );
	return ((T)proc)( col, wszName );
}

int		WINAPI	D3DPERF_EndEvent( void ) {
	typedef int (WINAPI *T)(void);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_EndEvent" );
	return ((T)proc)( );
}

DWORD	WINAPI	D3DPERF_GetStatus( void ) {
	typedef DWORD (WINAPI *T)(void);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_GetStatus" );
	return ((T)proc)( );
}

BOOL	WINAPI	D3DPERF_QueryRepeatFrame( void ) {
	typedef BOOL (WINAPI *T)(void);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_QueryRepeatFrame" );
	return ((T)proc)( );
}

void	WINAPI	D3DPERF_SetMarker( D3DCOLOR col, LPCWSTR wszName ) {
	typedef void (WINAPI *T)(D3DCOLOR, LPCWSTR);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_SetMarker" );
	((T)proc)( col, wszName );
}

void	WINAPI	D3DPERF_SetOptions( DWORD dwOptions ) {
	typedef void (WINAPI *T)(DWORD);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_SetOptions" );
	((T)proc)( dwOptions );
}

void	WINAPI	D3DPERF_SetRegion( D3DCOLOR col, LPCWSTR wszName ) {
	typedef void (WINAPI *T)(D3DCOLOR, LPCWSTR);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "D3DPERF_SetRegion" );
	((T)proc)( col, wszName );
}

void	WINAPI	DebugSetMute( void ) {
	typedef void (WINAPI *T)(void);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "DebugSetMute" );
	((T)proc)( );
}

IDirect3D9* WINAPI Direct3DCreate9( UINT SDKVersion ) {
	typedef IDirect3D9* (WINAPI *T)(UINT);
	FARPROC proc = GetProcAddress( ::g_hBackEnd, "Direct3DCreate9" );

	// wrap
	IDirect3D9 *const lpIDirect3D9 = ((T)proc)( SDKVersion );
	::g_lpIDirect3D9 = new proxIDirect3D9( lpIDirect3D9 );
	return ::g_lpIDirect3D9;
}
