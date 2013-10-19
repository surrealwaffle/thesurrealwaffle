#include "proxIDirect3D9Device.h"
#include "proxy.h"
#include "control.h"
#include "runner.h"
#include <stdio.h>

HRESULT proxIDirect3DDevice9::Present(CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
{ 
	::PollKeys( );
	
	if ( GetKeyEvent( KEY_F4 ) == KEYEVENT_PRESSED ) {
		::SubmitChat( L"Hello World!" );
	}

	// not very descriptive names, but im in a rush
	bool b = false;
	float3 p( 0.f, 0.f, 0.f );

	// honestly should just make this constant-time access
	if ( GetKeyEvent( KEY_F1 ) == KEYEVENT_PRESSED ) {
		b = true;
		p = float3( -3.8f, -0.8f, 0.f );
		::SubmitChat( L"going to red flag" );
	} else if ( GetKeyEvent( KEY_F2 ) == KEYEVENT_PRESSED ) {
		b = true;
		p = float3( 0.f, 0.f, 0.f );
		::SubmitChat( L"going to middle" );
	} else if ( GetKeyEvent( KEY_F3 ) == KEYEVENT_PRESSED ) {
		b = true;
		p = float3( 3.8f, -0.8f, 0.f );
		::SubmitChat( L"going to blue flag" );
	}

	if ( b ) {
		::SetObjective( p );

		wchar_t buf[0x100];
		wprintf( buf, "%1.1f,%1.1f,%1.1f", p[0], p[1], p[2] );
		::SubmitChat( buf );
	}

	switch ( ::UpdateRunner( ) ) {
	case RUNUPDATE_REACHED:
		::SubmitChat( L"I have reached the destination." );
		break;
	}

    // call original routine
	HRESULT hres = lpIDirect3DDevice9->Present( pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);

	return (hres);
}

HRESULT proxIDirect3DDevice9::QueryInterface(REFIID riid, void** ppvObj)
{
    // check if original dll can provide interface. then send *our* address
	*ppvObj = NULL;

	HRESULT hRes = lpIDirect3DDevice9->QueryInterface(riid, ppvObj); 

	if (hRes == NOERROR)
	{
		*ppvObj = this;
	}
	
	return hRes;
}

ULONG proxIDirect3DDevice9::AddRef(void)
{
    return(lpIDirect3DDevice9->AddRef());
}

ULONG proxIDirect3DDevice9::Release(void)
{
	// ATTENTION: This is a booby-trap ! Watch out !
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete that objects before calling the original
	// Release function	

    // release/delete own objects
    // .....
	
	// Calling original function now
	ULONG count = lpIDirect3DDevice9->Release();

    if(count==0)
        delete(this);  // destructor will be called automatically

	return (count);
}

HRESULT proxIDirect3DDevice9::TestCooperativeLevel(void)
{
    return(lpIDirect3DDevice9->TestCooperativeLevel());
}

UINT proxIDirect3DDevice9::GetAvailableTextureMem(void)
{
    return(lpIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT proxIDirect3DDevice9::EvictManagedResources(void)
{
    return(lpIDirect3DDevice9->EvictManagedResources());
}

HRESULT proxIDirect3DDevice9::GetDirect3D(IDirect3D9** ppD3D9)
{
    return(lpIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT proxIDirect3DDevice9::GetDeviceCaps(D3DCAPS9* pCaps)
{
    return(lpIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT proxIDirect3DDevice9::GetDisplayMode(UINT iSwapChain,D3DDISPLAYMODE* pMode)
{
    return(lpIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT proxIDirect3DDevice9::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters)
{
    return(lpIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT proxIDirect3DDevice9::SetCursorProperties(UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap)
{
    return(lpIDirect3DDevice9->SetCursorProperties(XHotSpot,YHotSpot,pCursorBitmap));
}

void    proxIDirect3DDevice9::SetCursorPosition(int X,int Y,DWORD Flags)
{
    return(lpIDirect3DDevice9->SetCursorPosition(X,Y,Flags));
}

BOOL    proxIDirect3DDevice9::ShowCursor(BOOL bShow)
{
    return(lpIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT proxIDirect3DDevice9::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain)  
{
    return(lpIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters,pSwapChain));
}

HRESULT proxIDirect3DDevice9::GetSwapChain(UINT iSwapChain,IDirect3DSwapChain9** pSwapChain)
{
    return(lpIDirect3DDevice9->GetSwapChain(iSwapChain,pSwapChain));
}

UINT    proxIDirect3DDevice9::GetNumberOfSwapChains(void)
{
    return(lpIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT proxIDirect3DDevice9::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
    return(lpIDirect3DDevice9->Reset(pPresentationParameters));
}

HRESULT proxIDirect3DDevice9::GetBackBuffer(UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer)
{
    return(lpIDirect3DDevice9->GetBackBuffer(iSwapChain,iBackBuffer,Type,ppBackBuffer));
}

HRESULT proxIDirect3DDevice9::GetRasterStatus(UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus)
{
    return(lpIDirect3DDevice9->GetRasterStatus(iSwapChain,pRasterStatus));
}

HRESULT proxIDirect3DDevice9::SetDialogBoxMode(BOOL bEnableDialogs)
{
    return(lpIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void    proxIDirect3DDevice9::SetGammaRamp(UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp)
{
    return(lpIDirect3DDevice9->SetGammaRamp(iSwapChain,Flags,pRamp));
}

void    proxIDirect3DDevice9::GetGammaRamp(UINT iSwapChain,D3DGAMMARAMP* pRamp)
{
    return(lpIDirect3DDevice9->GetGammaRamp(iSwapChain,pRamp));
}

HRESULT proxIDirect3DDevice9::CreateTexture(UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateTexture(Width,Height,Levels,Usage,Format,Pool,ppTexture,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateVolumeTexture(UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateVolumeTexture(Width,Height,Depth,Levels,Usage,Format,Pool,ppVolumeTexture,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateCubeTexture(UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateCubeTexture(EdgeLength,Levels,Usage,Format,Pool,ppCubeTexture,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateVertexBuffer(UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateVertexBuffer(Length,Usage,FVF,Pool,ppVertexBuffer,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateIndexBuffer(UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateIndexBuffer(Length,Usage,Format,Pool,ppIndexBuffer,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateRenderTarget(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint)
{
    return(lpIDirect3DDevice9->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint));
}

HRESULT proxIDirect3DDevice9::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture)
{
    return(lpIDirect3DDevice9->UpdateTexture(pSourceTexture,pDestinationTexture));
}

HRESULT proxIDirect3DDevice9::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface)
{
    return(lpIDirect3DDevice9->GetRenderTargetData(pRenderTarget,pDestSurface));
}

HRESULT proxIDirect3DDevice9::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface)
{
    return(lpIDirect3DDevice9->GetFrontBufferData(iSwapChain,pDestSurface));
}

HRESULT proxIDirect3DDevice9::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter)
{
    return(lpIDirect3DDevice9->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter));
}

HRESULT proxIDirect3DDevice9::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color)
{
    return(lpIDirect3DDevice9->ColorFill(pSurface,pRect,color));
}

HRESULT proxIDirect3DDevice9::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle)
{
    return(lpIDirect3DDevice9->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle));
}

HRESULT proxIDirect3DDevice9::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget)
{
    return(lpIDirect3DDevice9->SetRenderTarget(RenderTargetIndex,pRenderTarget));
}

HRESULT proxIDirect3DDevice9::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget)
{
    return(lpIDirect3DDevice9->GetRenderTarget(RenderTargetIndex,ppRenderTarget));
}

HRESULT proxIDirect3DDevice9::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil)
{
    return(lpIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT proxIDirect3DDevice9::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface)
{
    return(lpIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT proxIDirect3DDevice9::BeginScene(void)
{
    return(lpIDirect3DDevice9->BeginScene());
}

HRESULT proxIDirect3DDevice9::EndScene(void)
{
    return(lpIDirect3DDevice9->EndScene());
}

HRESULT proxIDirect3DDevice9::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil)
{
    return(lpIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT proxIDirect3DDevice9::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return(lpIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT proxIDirect3DDevice9::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix)
{
    return(lpIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT proxIDirect3DDevice9::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix)
{
    return(lpIDirect3DDevice9->MultiplyTransform(State,pMatrix));
}

HRESULT proxIDirect3DDevice9::SetViewport(CONST D3DVIEWPORT9* pViewport)
{
    return(lpIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT proxIDirect3DDevice9::GetViewport(D3DVIEWPORT9* pViewport)
{
    return(lpIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT proxIDirect3DDevice9::SetMaterial(CONST D3DMATERIAL9* pMaterial)
{
    return(lpIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT proxIDirect3DDevice9::GetMaterial(D3DMATERIAL9* pMaterial)
{
    return(lpIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT proxIDirect3DDevice9::SetLight(DWORD Index,CONST D3DLIGHT9* pLight)
{
    return(lpIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT proxIDirect3DDevice9::GetLight(DWORD Index,D3DLIGHT9* pLight)
{
    return(lpIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT proxIDirect3DDevice9::LightEnable(DWORD Index,BOOL Enable)
{
    return(lpIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT proxIDirect3DDevice9::GetLightEnable(DWORD Index,BOOL* pEnable)
{
    return(lpIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT proxIDirect3DDevice9::SetClipPlane(DWORD Index,CONST float* pPlane)
{
    return(lpIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT proxIDirect3DDevice9::GetClipPlane(DWORD Index,float* pPlane)
{
    return(lpIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT proxIDirect3DDevice9::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value)
{
    return(lpIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT proxIDirect3DDevice9::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue)
{
    return(lpIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT proxIDirect3DDevice9::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB)
{
    return(lpIDirect3DDevice9->CreateStateBlock(Type,ppSB));
}

HRESULT proxIDirect3DDevice9::BeginStateBlock(void)
{
    return(lpIDirect3DDevice9->BeginStateBlock());
}

HRESULT proxIDirect3DDevice9::EndStateBlock(IDirect3DStateBlock9** ppSB)
{
    return(lpIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT proxIDirect3DDevice9::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus)
{
    return(lpIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT proxIDirect3DDevice9::GetClipStatus(D3DCLIPSTATUS9* pClipStatus)
{
    return(lpIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT proxIDirect3DDevice9::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture)
{
    return(lpIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT proxIDirect3DDevice9::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
    return(lpIDirect3DDevice9->SetTexture(Stage,pTexture));
}

HRESULT proxIDirect3DDevice9::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
    return(lpIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT proxIDirect3DDevice9::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
    return(lpIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT proxIDirect3DDevice9::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
    return(lpIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT proxIDirect3DDevice9::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
    return(lpIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT proxIDirect3DDevice9::ValidateDevice(DWORD* pNumPasses)
{
    return(lpIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT proxIDirect3DDevice9::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
    return(lpIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT proxIDirect3DDevice9::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
    return(lpIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT proxIDirect3DDevice9::SetCurrentTexturePalette(UINT PaletteNumber)
{
    return(lpIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT proxIDirect3DDevice9::GetCurrentTexturePalette(UINT *PaletteNumber)
{
    return(lpIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT proxIDirect3DDevice9::SetScissorRect(CONST RECT* pRect)
{
    return(lpIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT proxIDirect3DDevice9::GetScissorRect( RECT* pRect)
{
    return(lpIDirect3DDevice9->GetScissorRect( pRect));
}

HRESULT proxIDirect3DDevice9::SetSoftwareVertexProcessing(BOOL bSoftware)
{
    return(lpIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    proxIDirect3DDevice9::GetSoftwareVertexProcessing(void)
{
    return(lpIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT proxIDirect3DDevice9::SetNPatchMode(float nSegments)
{
    return(lpIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   proxIDirect3DDevice9::GetNPatchMode(void)
{
    return(lpIDirect3DDevice9->GetNPatchMode());
}

HRESULT proxIDirect3DDevice9::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
    return(lpIDirect3DDevice9->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount));
}

HRESULT proxIDirect3DDevice9::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
    return(lpIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount));
}

HRESULT proxIDirect3DDevice9::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return(lpIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT proxIDirect3DDevice9::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
    return(lpIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT proxIDirect3DDevice9::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
    return(lpIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT proxIDirect3DDevice9::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
    return(lpIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
}

HRESULT proxIDirect3DDevice9::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
    return(lpIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT proxIDirect3DDevice9::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
    return(lpIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT proxIDirect3DDevice9::SetFVF(DWORD FVF)
{
    return(lpIDirect3DDevice9->SetFVF(FVF));
}

HRESULT proxIDirect3DDevice9::GetFVF(DWORD* pFVF)
{
    return(lpIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT proxIDirect3DDevice9::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
    return(lpIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}

HRESULT proxIDirect3DDevice9::SetVertexShader(IDirect3DVertexShader9* pShader)
{
    return(lpIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT proxIDirect3DDevice9::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
    return(lpIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT proxIDirect3DDevice9::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return(lpIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT proxIDirect3DDevice9::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return(lpIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT proxIDirect3DDevice9::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return(lpIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT proxIDirect3DDevice9::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return(lpIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT proxIDirect3DDevice9::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return(lpIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT proxIDirect3DDevice9::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return(lpIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT proxIDirect3DDevice9::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
    return(lpIDirect3DDevice9->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride));
}

HRESULT proxIDirect3DDevice9::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
    return(lpIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT proxIDirect3DDevice9::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
    return(lpIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT proxIDirect3DDevice9::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
    return(lpIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT proxIDirect3DDevice9::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
    return(lpIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT proxIDirect3DDevice9::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
    return(lpIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT proxIDirect3DDevice9::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
    return(lpIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT proxIDirect3DDevice9::SetPixelShader(IDirect3DPixelShader9* pShader)
{
    return(lpIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT proxIDirect3DDevice9::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
    return(lpIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT proxIDirect3DDevice9::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
    return(lpIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT proxIDirect3DDevice9::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
    return(lpIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT proxIDirect3DDevice9::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
    return(lpIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT proxIDirect3DDevice9::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
    return(lpIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT proxIDirect3DDevice9::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
    return(lpIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT proxIDirect3DDevice9::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
    return(lpIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT proxIDirect3DDevice9::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
    return(lpIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT proxIDirect3DDevice9::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
    return(lpIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT proxIDirect3DDevice9::DeletePatch(UINT Handle)
{
    return(lpIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT proxIDirect3DDevice9::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
    return(lpIDirect3DDevice9->CreateQuery(Type,ppQuery));
}
