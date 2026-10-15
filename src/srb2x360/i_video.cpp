#include "../platform/stdafx.h"
#include "../render/renderer.h"
#include "../platform/quadrender.h"
extern "C" {
#include "../../srb2/src/doomdef.h"
#include "../../srb2/src/command.h"
#include "../../srb2/src/i_video.h"
#include "../../srb2/src/screen.h"
#include "../../srb2/src/i_system.h"
}

rendermode_t rendermode = render_soft;
boolean highcolor = false;
boolean allow_fullscreen = false;
UINT8 graphics_started = 0;
static RGBA_t s_palette[256];
static D3DTexture *s_frame = NULL;
static const int kW=640, kH=400;
consvar_t cv_vidwait = {"vid_wait", "On", CV_SAVE, CV_OnOff, NULL, 0, NULL, NULL, 0, 0, NULL};

static void EnsureTexture()
{
    if (!s_frame && g_pd3dDevice)
    {
        HRESULT hr = g_pd3dDevice->CreateTexture(kW, kH, 1, 0,
        D3DFMT_LIN_A8R8G8B8, 0, &s_frame, NULL);
        if (FAILED(hr) || !s_frame)
        I_Error("Xbox 360: failed to create linear framebuffer texture (hr=0x%08x)", (unsigned)hr);
    }
}

void I_StartupGraphics(void)
{
    if (graphics_started) return;
    CV_RegisterVar(&cv_vidwait);
    vid.width=kW; vid.height=kH; vid.bpp=1; vid.rowbytes=kW; vid.modenum=0;
    vid.recalc=true; vid.direct=NULL; vid.WndParent=NULL;
    vid.buffer=(UINT8*)malloc((size_t)kW*kH*NUMSCREENS);
    if (!vid.buffer) I_Error("Xbox 360: video buffer allocation failed\n");
    memset(vid.buffer,0,(size_t)kW*kH*NUMSCREENS);
    EnsureTexture();
    graphics_started=1;
}
void I_ShutdownGraphics(void){ if(s_frame){s_frame->Release();s_frame=NULL;} if(vid.buffer){free(vid.buffer);vid.buffer=NULL;} graphics_started=0; }
void I_SetPalette(RGBA_t *p){ if(p) memcpy(s_palette,p,sizeof(s_palette)); }
INT32 VID_NumModes(void){return 1;}
INT32 VID_GetModeForSize(INT32 w,INT32 h){(void)w;(void)h;return 0;}
void VID_PrepareModeList(void){}
INT32 VID_SetMode(INT32 m){(void)m; if(!graphics_started) I_StartupGraphics(); vid.recalc=true; return 1;}
const char *VID_GetModeName(INT32 m){(void)m;return "640x400";}
void I_UpdateNoBlit(void){}

static void PresentSoftwareFrame()
{
    if(!graphics_started || !vid.buffer || !g_pd3dDevice) return;
    EnsureTexture(); if(!s_frame) return;
    D3DLOCKED_RECT lr;
    HRESULT lockResult = s_frame->LockRect(0, &lr, NULL, 0);
    if (SUCCEEDED(lockResult)) {
        if (!lr.pBits || lr.Pitch < kW * (int)sizeof(DWORD)) {
            s_frame->UnlockRect(0);
            I_Error("Xbox 360: invalid linear framebuffer lock (pitch=%d)", (int)lr.Pitch);
            return;
        }
        for(int y=0;y<kH;y++) {
            DWORD *dst=(DWORD*)((BYTE*)lr.pBits + y*lr.Pitch);
            const UINT8 *src=vid.buffer + y*vid.rowbytes;
            for(int x=0;x<kW;x++) {
                RGBA_t c=s_palette[src[x]];
                dst[x]=0xff000000u | ((DWORD)c.s.red<<16) | ((DWORD)c.s.green<<8) | (DWORD)c.s.blue;
            }
        }
        s_frame->UnlockRect(0);
    } else {
        I_Error("Xbox 360: framebuffer LockRect failed (hr=0x%08x)", (unsigned)lockResult);
        return;
    }
    BeginFrame(0xff000000);
    DrawTexturedQuad(g_pd3dDevice,s_frame,640.0f,360.0f,576.0f,360.0f,0.0f);
    EndFrame();
}
void I_FinishUpdate(void){PresentSoftwareFrame();}
void I_UpdateNoVsync(void){PresentSoftwareFrame();}
void I_WaitVBL(INT32 count){ if(count>0) Sleep((DWORD)(count*1000/60)); }
void I_ReadScreen(UINT8 *scr){ if(scr&&vid.buffer) memcpy(scr,vid.buffer,(size_t)vid.rowbytes*vid.height); }
void I_BeginRead(void){}
void I_EndRead(void){}
