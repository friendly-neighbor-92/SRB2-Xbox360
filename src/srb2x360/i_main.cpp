#include "../platform/stdafx.h"
extern "C" {
#include "../../srb2/src/doomdef.h"
#include "../../srb2/src/d_main.h"
#include "../../srb2/src/m_argv.h"
}
#include "../render/renderer.h"
#include "../platform/quadrender.h"
#include "fatal_screen.h"
#include <stdio.h>

static void BootStatus(const char *message, DWORD color)
{
    OutputDebugStringA(message);
    OutputDebugStringA("\n");
    FILE *f = fopen("game:\\srb2_boot.log", "ab");
    if (f) { fprintf(f, "%s\r\n", message); fclose(f); }
    if (g_pd3dDevice) SRB2_DrawLoadingScreen(color);
}

void __cdecl main()
{
    static char arg0[] = "game:\\default.xex";
    static char arg1[] = "-home";
    static char arg2[] = "game:\\";
    static char *argv[] = { arg0, arg1, arg2, NULL };
    myargc = 3;
    myargv = argv;
    if (FAILED(InitD3D())) { OutputDebugStringA("SRB2: InitD3D failed\n"); for (;;) Sleep(1000); }
    InitQuadRenderer(g_pd3dDevice);
    BootStatus("SRB2: D3D initialized; blue means initializing game", 0xff003080);
    BootStatus("SRB2: quad renderer initialized; checking IWAD", 0xff204080);
    {
        const char *files[] = { "srb2.wad", "srb2.srb", "zones.dta", "player.dta", "rings.dta", "patch.dta" };
        for (unsigned i=0; i<sizeof(files)/sizeof(files[0]); ++i) {
            char path[128]; sprintf(path, "game:\\%s", files[i]);
            FILE *f = fopen(path,"rb");
            if (f) { fclose(f); BootStatus(path, 0xff204080); }
            else { OutputDebugStringA("SRB2 missing/inaccessible: "); OutputDebugStringA(path); OutputDebugStringA("\n"); }
        }
    }
    BootStatus("SRB2: entering D_SRB2Main", 0xff405080);
    D_SRB2Main();
    BootStatus("SRB2: entering gameplay loop", 0xff205020);
    D_SRB2Loop();
    for (;;) Sleep(1000);
}
