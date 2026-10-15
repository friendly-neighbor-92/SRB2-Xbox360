#pragma once

// Shared textured-quad renderer.
const float SCREEN_W = 1280.0f;
const float SCREEN_H = 720.0f;

// Vertex

struct SCREEN_VERTEX
{
    FLOAT x;
    FLOAT y;
    FLOAT z;
    FLOAT w;

    DWORD color;

    FLOAT u;
    FLOAT v;
};

// D3D resources

extern D3DVertexDeclaration* g_pQuadDecl;
extern D3DVertexShader*       g_pQuadVS;
extern D3DPixelShader*        g_pQuadPS;

// Lifecycle

void InitQuadRenderer(
D3DDevice* pd3dDevice
);

void ShutdownQuadRenderer();

// Normal textured quad

void DrawTexturedQuad(
D3DDevice*  pd3dDevice,
D3DTexture* pTexture,

float cx,
float cy,

float hw,
float hh,

float angle = 0.0f
);

// UV textured quad
// Same as DrawTexturedQuad, but lets the caller select a portion of the
// texture.
// UV coordinates:
//     0,0 = top-left
//     1,1 = bottom-right

void DrawTexturedQuadUV(
D3DDevice*  pd3dDevice,
D3DTexture* pTexture,

float cx,
float cy,

float hw,
float hh,

float u0,
float v0,
float u1,
float v1,

DWORD color,

float angle = 0.0f
);
