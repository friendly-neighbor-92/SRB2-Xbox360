#include "../platform/stdafx.h"
#include "../platform/quadrender.h"
#include "../platform/font_fallback.h"
#include "../render/renderer.h"
#include "fatal_screen.h"

static D3DTexture *s_font = NULL;
static bool PrepareFont()
{
    if (s_font) return true;
    if (!g_pd3dDevice || FAILED(D3DXCreateTexture(g_pd3dDevice, 256, 128, 1, 0,
    D3DFMT_LIN_A8R8G8B8, D3DPOOL_MANAGED, &s_font)) || !s_font) return false;
    D3DLOCKED_RECT lr;
    if (FAILED(s_font->LockRect(0, &lr, NULL, 0))) { s_font->Release(); s_font = NULL; return false; }
    for (int c=0;c<128;++c) for (int y=0;y<16;++y) {
        DWORD *dst=(DWORD*)((BYTE*)lr.pBits + ((c/16)*16+y)*lr.Pitch)+(c%16)*16;
        const unsigned short bits=FONT_FALLBACK_BITS[c*16+y];
        for(int x=0;x<16;++x) dst[x]=(bits & (0x8000u>>x))?0xffffffffu:0x00ffffffu;
    }
    s_font->UnlockRect(0);
    return true;
}
static void Line(const char *text, int length, float x, float y, DWORD color)
{
    if (!s_font) return;
    for (int i=0; i<length; ++i) {
        unsigned char c=(unsigned char)text[i];
        if(c<32 || c>=127) c='?';
        float u=(float)(c%16)/16.0f, v=(float)(c/16)/8.0f;
        DrawTexturedQuadUV(g_pd3dDevice,s_font,x+i*13.0f+8.0f,y+10.0f,
        8.0f,10.0f,u,v,u+1.0f/16.0f,v+1.0f/8.0f,color);
    }
}
void SRB2_DrawFatalScreen(const char *message)
{
    if (!g_pd3dDevice) return;
    const bool fontReady=PrepareFont();
    BeginFrame(0xff480c13u);
    if (fontReady) {
        Line("SRB2 XBOX 360 - FATAL ERROR",sizeof("SRB2 XBOX 360 - FATAL ERROR")-1,44,42,0xffffffffu);
        Line("Exact engine message:",sizeof("Exact engine message:")-1,44,91,0xffffdf99u);
        const char *p=message?message:"(no error message)";
        float y=130.0f;
        while (*p && y<650.0f) {
            char line[88]; int n=0;
            while (*p && *p!='\n' && *p!='\r' && n<87) {
                unsigned char c=(unsigned char)*p++;
                line[n++]=(c>=32 && c<127)?(char)c:'?';
            }
            Line(line,n,44,y,0xffffffffu);
            y+=25.0f;
            if (*p=='\r') ++p;
            if (*p=='\n') ++p;
        }
        if (*p) Line("[MESSAGE TRUNCATED - SEE DEBUG LOG]",sizeof("[MESSAGE TRUNCATED - SEE DEBUG LOG]")-1,44,670,0xffffdf99u);
    }
    EndFrame();
}

void SRB2_DrawLoadingScreen(DWORD color)
{
    if (!g_pd3dDevice) return;
    const bool fontReady = PrepareFont();
    BeginFrame(color);
    if (fontReady) {
        static const char label[] = "Loading, please wait...";
        Line(label, sizeof(label)-1, 44, 80, 0xffffffffu);
    }
    EndFrame();
}
