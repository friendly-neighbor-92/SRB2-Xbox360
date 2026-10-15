#pragma once
#include <d3dx9.h>
extern D3DDevice* g_pd3dDevice;
extern BOOL       g_bWidescreen;

HRESULT InitD3D();                     // device, display resolution up to 720p
void    BeginFrame( DWORD clearColor ); // Clear + default state
void    EndFrame();                    // Present

// Compile helper shared by the pipelines (returns NULL on failure).
ID3DXBuffer* CompileShader( const char* src, const char* profile );
