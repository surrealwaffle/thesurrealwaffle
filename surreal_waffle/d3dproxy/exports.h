#ifndef EXPORTS_H
#define EXPORTS_H

#include "stdafx.h"

/*
	exports.h:
		Declaration and definition of exported functions:
			--CheckFullscreen
			Direct3DShaderValidatorCreate9
			--PSGPError
			--PSGPSampleTexture
			D3DPERF_BeginEvent
			D3DPERF_EndEvent
			D3DPERF_GetStatus
			D3DPERF_QueryRepeatFrame
			D3DPERF_SetMarker
			D3DPERF_SetOptions
			D3DPERF_SetRegion
			--DebugSetLevel
			DebugSetMute
			Direct3DCreate9
*/

void	WINAPI	Direct3DShaderValidatorCreate9( void );
int		WINAPI	D3DPERF_BeginEvent( D3DCOLOR col, LPCWSTR wszName );
int		WINAPI	D3DPERF_EndEvent( void );
DWORD	WINAPI	D3DPERF_GetStatus( void );
BOOL	WINAPI	D3DPERF_QueryRepeatFrame( void );
void	WINAPI	D3DPERF_SetMarker( D3DCOLOR col, LPCWSTR wszName );
void	WINAPI	D3DPERF_SetOptions( DWORD dwOptions );
void	WINAPI	D3DPERF_SetRegion( D3DCOLOR col, LPCWSTR wszName );
void	WINAPI	DebugSetMute( void );
IDirect3D9* WINAPI Direct3DCreate9( UINT SDKVersion );

#endif // EXPORTS_H