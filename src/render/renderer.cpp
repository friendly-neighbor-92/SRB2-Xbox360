#include "stdafx.h"
#include "renderer.h"

D3DDevice* g_pd3dDevice  = NULL;
BOOL       g_bWidescreen = TRUE;

HRESULT InitD3D()
{
    Direct3D* pD3D = Direct3DCreate9( D3D_SDK_VERSION );
    if( !pD3D )
    return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory( &d3dpp, sizeof( d3dpp ) );

    XVIDEO_MODE VideoMode;
    XGetVideoMode( &VideoMode );
    g_bWidescreen = VideoMode.fIsWideScreen;

    d3dpp.BackBufferWidth        = min( (int)VideoMode.dwDisplayWidth,  1280 );
    d3dpp.BackBufferHeight       = min( (int)VideoMode.dwDisplayHeight,  720 );
    d3dpp.BackBufferFormat       = D3DFMT_X8R8G8B8;
    d3dpp.BackBufferCount        = 1;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D24S8;
    d3dpp.SwapEffect             = D3DSWAPEFFECT_DISCARD;
    d3dpp.PresentationInterval   = D3DPRESENT_INTERVAL_ONE;

    if( FAILED( pD3D->CreateDevice( 0, D3DDEVTYPE_HAL, NULL,
    D3DCREATE_HARDWARE_VERTEXPROCESSING,
    &d3dpp, &g_pd3dDevice ) ) )
    return E_FAIL;

    return S_OK;
}

void BeginFrame( DWORD clearColor )
{
    g_pd3dDevice->Clear( 0L, NULL,
    D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER | D3DCLEAR_STENCIL,
    clearColor, 1.0f, 0L );

    g_pd3dDevice->SetRenderState( D3DRS_CULLMODE,         D3DCULL_NONE );
    g_pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,     TRUE );
    g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
}

void EndFrame()
{
    g_pd3dDevice->Present( NULL, NULL, NULL, NULL );
}

ID3DXBuffer* CompileShader( const char* src, const char* profile )
{
    ID3DXBuffer* pCode  = NULL;
    ID3DXBuffer* pError = NULL;

    HRESULT hr = D3DXCompileShader( src, (UINT)strlen( src ), NULL, NULL,
    "main", profile, 0, &pCode, &pError, NULL );
    if( FAILED( hr ) )
    {
        if( pError )
        {
            OutputDebugStringA( (char*)pError->GetBufferPointer() );
            pError->Release();
        }
        return NULL;
    }
    if( pError ) pError->Release();
    return pCode;
}
