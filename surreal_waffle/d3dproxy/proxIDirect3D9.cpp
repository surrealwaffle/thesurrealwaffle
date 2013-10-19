#include "proxIDirect3D9.h"
#include "proxy.h"

HRESULT proxIDirect3D9::QueryInterface( REFIID riid, void** ppvObj ) {
	HRESULT const hQuery = this->lpIDirect3D9->QueryInterface( riid, ppvObj );

	if ( hQuery == NOERROR )
		*ppvObj = this;

	return hQuery;
}

ULONG proxIDirect3D9::AddRef( void ) {
	return this->lpIDirect3D9->AddRef( );
}

ULONG proxIDirect3D9::Release( void ) {
	ULONG const refn = this->lpIDirect3D9->Release( );
	if ( refn == 0 )
		delete this;

	return refn;
}

HRESULT proxIDirect3D9::CreateDevice( UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface ) {
	IDirect3DDevice9* lpDev = NULL;
	HRESULT hRes = lpIDirect3D9->CreateDevice( Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, &lpDev );

	::g_lpIDirect3DDevice9 = new proxIDirect3DDevice9( lpDev );
	*ppReturnedDeviceInterface = ::g_lpIDirect3DDevice9;

	return hRes;
}

HRESULT proxIDirect3D9::RegisterSoftwareDevice( void* pInitializeFunction ) {
	return lpIDirect3D9->RegisterSoftwareDevice( pInitializeFunction );
}
UINT proxIDirect3D9::GetAdapterCount( void ) {
	return lpIDirect3D9->GetAdapterCount( );
}
HRESULT proxIDirect3D9::GetAdapterIdentifier( UINT Adapter, DWORD Flags, D3DADAPTER_IDENTIFIER9* pIdentifier ) {
	return lpIDirect3D9->GetAdapterIdentifier( Adapter, Flags, pIdentifier );
}
UINT proxIDirect3D9::GetAdapterModeCount( UINT Adapter, D3DFORMAT Format ){
	return lpIDirect3D9->GetAdapterModeCount( Adapter, Format );
}
HRESULT proxIDirect3D9::EnumAdapterModes( UINT Adapter, D3DFORMAT Format, UINT Mode, D3DDISPLAYMODE* pMode ) {
	return lpIDirect3D9->EnumAdapterModes( Adapter, Format, Mode, pMode );
}
HRESULT proxIDirect3D9::GetAdapterDisplayMode( UINT Adapter, D3DDISPLAYMODE* pMode ) {
	return lpIDirect3D9->GetAdapterDisplayMode( Adapter, pMode );
}
HRESULT proxIDirect3D9::CheckDeviceType( UINT Adapter, D3DDEVTYPE DevType, D3DFORMAT DisplayFormat, D3DFORMAT BackBufferFormat, BOOL bWindowed ) {
	return lpIDirect3D9->CheckDeviceType( Adapter, DevType, DisplayFormat, BackBufferFormat, bWindowed );
}
HRESULT proxIDirect3D9::CheckDeviceFormat( UINT Adapter,D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, DWORD Usage, D3DRESOURCETYPE RType, D3DFORMAT CheckFormat ) {
	return lpIDirect3D9->CheckDeviceFormat( Adapter, DeviceType, AdapterFormat, Usage, RType, CheckFormat );
}
HRESULT proxIDirect3D9::CheckDeviceMultiSampleType( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SurfaceFormat, BOOL Windowed, D3DMULTISAMPLE_TYPE MultiSampleType, DWORD* pQualityLevels ) {
	return lpIDirect3D9->CheckDeviceMultiSampleType( Adapter, DeviceType, SurfaceFormat, Windowed, MultiSampleType, pQualityLevels );
}
HRESULT proxIDirect3D9::CheckDepthStencilMatch( UINT Adapter,D3DDEVTYPE DeviceType, D3DFORMAT AdapterFormat, D3DFORMAT RenderTargetFormat, D3DFORMAT DepthStencilFormat ) {
	return lpIDirect3D9->CheckDepthStencilMatch( Adapter, DeviceType, AdapterFormat, RenderTargetFormat, DepthStencilFormat );
}
HRESULT proxIDirect3D9::CheckDeviceFormatConversion( UINT Adapter, D3DDEVTYPE DeviceType, D3DFORMAT SourceFormat, D3DFORMAT TargetFormat ) {
	return lpIDirect3D9->CheckDeviceFormatConversion( Adapter, DeviceType, SourceFormat, TargetFormat );
}
HRESULT proxIDirect3D9::GetDeviceCaps( UINT Adapter, D3DDEVTYPE DeviceType, D3DCAPS9* pCaps) {
	return lpIDirect3D9->GetDeviceCaps( Adapter, DeviceType, pCaps );
}
HMONITOR proxIDirect3D9::GetAdapterMonitor( UINT Adapter ) {
	return lpIDirect3D9->GetAdapterMonitor( Adapter );
}