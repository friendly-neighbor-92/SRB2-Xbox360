#pragma once
#include <d3dx9.h>
extern D3DDevice* g_pd3dDevice;
extern BOOL       g_bWidescreen;

HRESULT InitD3D();                     // device, 1280x720, D24S8
void    BeginFrame( DWORD clearColor ); // Clear + default state
void    EndFrame();                    // Present

// Compile helper shared by the pipelines (returns NULL on failure).
ID3DXBuffer* CompileShader( const char* src, const char* profile );
