// Screen-space textured quad renderer.

#include "stdafx.h"

#include "quadrender.h"

D3DVertexDeclaration* g_pQuadDecl = NULL;
D3DVertexShader*      g_pQuadVS   = NULL;
D3DPixelShader*       g_pQuadPS   = NULL;
float SCREEN_W = 1280.0f;
float SCREEN_H = 720.0f;

void SetQuadScreenSize(float width, float height)
{
    if (width > 0.0f && height > 0.0f)
    {
        SCREEN_W = width;
        SCREEN_H = height;
    }
}

static const char* s_QuadVS =
" struct VS_IN  "
" { "
"     float4 pos : POSITION; "
"     float4 col : COLOR0; "
"     float2 uv  : TEXCOORD0; "
" }; "

" struct VS_OUT "
" { "
"     float4 pos : POSITION; "
"     float4 col : COLOR0; "
"     float2 uv  : TEXCOORD0; "
" }; "

" VS_OUT main( VS_IN i ) "
" { "
"     VS_OUT o; "
"     o.pos = i.pos; "
"     o.col = i.col; "
"     o.uv  = i.uv; "
"     return o; "
" } ";

static const char* s_QuadPS =
" sampler2D g_tex : register(s0); "

" struct PS_IN "
" { "
"     float4 col : COLOR0; "
"     float2 uv  : TEXCOORD0; "
" }; "

" float4 main( PS_IN i ) : COLOR "
" { "
"     return tex2D( g_tex, i.uv ) * i.col; "
" } ";

static inline void SC(
float sx,
float sy,
float& cx,
float& cy
)
{
    cx =
    ( sx / SCREEN_W ) *
    2.0f -
    1.0f;

    cy =
    -( sy / SCREEN_H ) *
    2.0f +
    1.0f;
}

void InitQuadRenderer(
D3DDevice* pd3dDevice
)
{
    if( !pd3dDevice )
    return;

    D3DVERTEXELEMENT9 elements[] =
    {
        {
            0,
            0,
            D3DDECLTYPE_FLOAT4,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_POSITION,
            0
        },

        {
            0,
            16,
            D3DDECLTYPE_D3DCOLOR,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_COLOR,
            0
        },

        {
            0,
            20,
            D3DDECLTYPE_FLOAT2,
            D3DDECLMETHOD_DEFAULT,
            D3DDECLUSAGE_TEXCOORD,
            0
        },

        D3DDECL_END()
    };

    pd3dDevice->CreateVertexDeclaration(
    elements,
    &g_pQuadDecl
    );

    ID3DXBuffer* pCode =
    NULL;

    HRESULT hr =
    D3DXCompileShader(
    s_QuadVS,
    (UINT)strlen(s_QuadVS),

    NULL,
    NULL,

    "main",
    "vs_2_0",

    0,

    &pCode,
    NULL,
    NULL
    );

    if( SUCCEEDED(hr) )
    {
        pd3dDevice->CreateVertexShader(
        (DWORD*)pCode->GetBufferPointer(),
        &g_pQuadVS
        );

        pCode->Release();
    }
    else
    {
        OutputDebugStringA(
        "ERROR: quad vertex shader failed\n"
        );
    }

    hr =
    D3DXCompileShader(
    s_QuadPS,
    (UINT)strlen(s_QuadPS),

    NULL,
    NULL,

    "main",
    "ps_2_0",

    0,

    &pCode,
    NULL,
    NULL
    );

    if( SUCCEEDED(hr) )
    {
        pd3dDevice->CreatePixelShader(
        (DWORD*)pCode->GetBufferPointer(),
        &g_pQuadPS
        );

        pCode->Release();
    }
    else
    {
        OutputDebugStringA(
        "ERROR: quad pixel shader failed\n"
        );
    }
}

void ShutdownQuadRenderer()
{
    if( g_pQuadDecl )
    {
        g_pQuadDecl->Release();
        g_pQuadDecl = NULL;
    }

    if( g_pQuadVS )
    {
        g_pQuadVS->Release();
        g_pQuadVS = NULL;
    }

    if( g_pQuadPS )
    {
        g_pQuadPS->Release();
        g_pQuadPS = NULL;
    }
}

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

float angle
)
{
    if(
    !pd3dDevice ||
    !pTexture ||
    !g_pQuadDecl ||
    !g_pQuadVS ||
    !g_pQuadPS
    )
    return;

    float cosA =
    cosf(angle);

    float sinA =
    sinf(angle);

    float ox[4] =
    {
        -hw,
        hw,
        -hw,
        hw
    };

    float oy[4] =
    {
        -hh,
        -hh,
        hh,
        hh
    };

    float clipX[4];
    float clipY[4];

    for(
    int i = 0;
    i < 4;
    ++i
    )
    {
        float rx =
        ox[i] * cosA -
        oy[i] * sinA;

        float ry =
        ox[i] * sinA +
        oy[i] * cosA;

        SC(
        cx + rx,
        cy + ry,

        clipX[i],
        clipY[i]
        );
    }

    SCREEN_VERTEX verts[4] =
    {
        {
            clipX[0],
            clipY[0],
            0.0f,
            1.0f,

            color,

            u0,
            v0
        },

        {
            clipX[1],
            clipY[1],
            0.0f,
            1.0f,

            color,

            u1,
            v0
        },

        {
            clipX[2],
            clipY[2],
            0.0f,
            1.0f,

            color,

            u0,
            v1
        },

        {
            clipX[3],
            clipY[3],
            0.0f,
            1.0f,

            color,

            u1,
            v1
        }
    };

    pd3dDevice->SetRenderState(
    D3DRS_ALPHABLENDENABLE,
    TRUE
    );

    pd3dDevice->SetRenderState(
    D3DRS_SRCBLEND,
    D3DBLEND_SRCALPHA
    );

    pd3dDevice->SetRenderState(
    D3DRS_DESTBLEND,
    D3DBLEND_INVSRCALPHA
    );

    pd3dDevice->SetRenderState(
    D3DRS_ZENABLE,
    FALSE
    );

    pd3dDevice->SetRenderState(
    D3DRS_ZWRITEENABLE,
    FALSE
    );

    pd3dDevice->SetSamplerState(
    0,
    D3DSAMP_MINFILTER,
    D3DTEXF_POINT
    );

    pd3dDevice->SetSamplerState(
    0,
    D3DSAMP_MAGFILTER,
    D3DTEXF_POINT
    );

    pd3dDevice->SetSamplerState(
    0,
    D3DSAMP_MIPFILTER,
    D3DTEXF_NONE
    );

    pd3dDevice->SetTexture(
    0,
    pTexture
    );

    pd3dDevice->SetVertexDeclaration(
    g_pQuadDecl
    );

    pd3dDevice->SetVertexShader(
    g_pQuadVS
    );

    pd3dDevice->SetPixelShader(
    g_pQuadPS
    );

    pd3dDevice->DrawPrimitiveUP(
    D3DPT_TRIANGLESTRIP,
    2,
    verts,
    sizeof(SCREEN_VERTEX)
    );

    pd3dDevice->SetRenderState(
    D3DRS_ALPHABLENDENABLE,
    FALSE
    );

    pd3dDevice->SetRenderState(
    D3DRS_ZENABLE,
    TRUE
    );

    pd3dDevice->SetRenderState(
    D3DRS_ZWRITEENABLE,
    TRUE
    );
}

// Existing API preserved.

void DrawTexturedQuad(
D3DDevice*  pd3dDevice,
D3DTexture* pTexture,

float cx,
float cy,

float hw,
float hh,

float angle
)
{
    DrawTexturedQuadUV(
    pd3dDevice,
    pTexture,

    cx,
    cy,

    hw,
    hh,

    0.0f,
    0.0f,

    1.0f,
    1.0f,

    0xFFFFFFFF,

    angle
    );
}
