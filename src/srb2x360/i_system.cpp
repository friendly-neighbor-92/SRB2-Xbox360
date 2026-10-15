#include "../platform/stdafx.h"
#include <stdarg.h>
#include <stdio.h>
#include "fatal_screen.h"
extern "C" {
#include "../../srb2/src/doomdef.h"
#include "../../srb2/src/i_system.h"
#include "../../srb2/src/d_main.h"
#include "../../srb2/src/g_input.h"
#include "../../srb2/src/doomstat.h"
#include "../../srb2/src/keys.h"
#include "../../srb2/src/m_argv.h"
#include "../../srb2/src/m_misc.h"
#include "../../srb2/src/g_game.h"
}

UINT8 keyboard_started=0;
static DWORD s_start=0;
static WORD s_prev[2] = {0,0};
static bool s_prevLT[2] = {false,false}, s_prevRT[2] = {false,false};
static void PostKey(INT32 key, bool down){event_t e; e.type=down?ev_keydown:ev_keyup;e.data1=key;e.data2=e.data3=0;D_PostEvent(&e);}
static void ButtonChanges(unsigned pad, WORD cur, WORD mask, INT32 key)
{
    const bool down=(cur&mask)!=0, old=(s_prev[pad]&mask)!=0;
    if (down!=old) PostKey(key,down);
}

UINT32 I_GetFreeMem(UINT32 *total){ if(total)*total=256u*1024u*1024u; return 128u*1024u*1024u; }
tic_t I_GetTime(void){DWORD now=GetTickCount(); if(!s_start)s_start=now; return (tic_t)(((UINT64)(now-s_start)*TICRATE)/1000);}
void I_Sleep(void){Sleep(1);}
void I_GetEvent(void)
{
    for (unsigned pad=0; pad<2; ++pad)
    {
        XINPUT_STATE st; ZeroMemory(&st,sizeof(st));
        if (XInputGetState(pad,&st)!=ERROR_SUCCESS) ZeroMemory(&st,sizeof(st));
        const WORD cur=st.Gamepad.wButtons;
        const INT32 joy=(pad==0)?KEY_JOY1:KEY_2JOY1;
        const INT32 hat=(pad==0)?KEY_HAT1:KEY_2HAT1;
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_A,joy+0);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_B,joy+1);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_X,joy+2);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_Y,joy+3);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_LEFT_SHOULDER,joy+6);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_RIGHT_SHOULDER,joy+7);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_START,joy+8);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_BACK,joy+9);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_DPAD_UP,hat+0);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_DPAD_DOWN,hat+1);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_DPAD_LEFT,hat+2);
        ButtonChanges(pad,cur,XINPUT_GAMEPAD_DPAD_RIGHT,hat+3);
        const bool lt=st.Gamepad.bLeftTrigger>XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        const bool rt=st.Gamepad.bRightTrigger>XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
        if(lt!=s_prevLT[pad]) PostKey(joy+4,lt);
        if(rt!=s_prevRT[pad]) PostKey(joy+5,rt);
        s_prevLT[pad]=lt; s_prevRT[pad]=rt;
        event_t e; e.type=(pad==0)?ev_joystick:ev_joystick2;
        e.data1=0; e.data2=st.Gamepad.sThumbLX/32; e.data3=-(st.Gamepad.sThumbLY/32); D_PostEvent(&e);
        e.data1=1; e.data2=st.Gamepad.sThumbRX/32; e.data3=-(st.Gamepad.sThumbRY/32); D_PostEvent(&e);
        s_prev[pad]=cur;
    }
}
void I_OsPolling(void){I_GetEvent();}
ticcmd_t *I_BaseTiccmd(void){static ticcmd_t c;memset(&c,0,sizeof(c));return &c;}
ticcmd_t *I_BaseTiccmd2(void){static ticcmd_t c;memset(&c,0,sizeof(c));return &c;}
void I_Quit(void)
{
    static bool saving = false;
    if (!saving)
    {
        saving = true;
        OutputDebugStringA("SRB2 SAVE: flushing config and gamedata to game:\\\n");
        M_SaveConfig(NULL);
        G_SaveGameData();
        OutputDebugStringA("SRB2 SAVE: normal quit flush complete\n");
    }
    OutputDebugStringA("SRB2: requesting dashboard launch\n");
    XLaunchNewImage(XLAUNCH_KEYWORD_DASH_ARCADE, NULL);
    OutputDebugStringA("SRB2: dashboard launch returned unexpectedly\n");
    for (;;) Sleep(1000);
}
void I_Error(const char *fmt,...){
    char b[2048]; va_list ap; va_start(ap,fmt);
    vsnprintf(b,sizeof(b)-1,fmt,ap); va_end(ap); b[sizeof(b)-1]=0;
    OutputDebugStringA("SRB2 FATAL: "); OutputDebugStringA(b); OutputDebugStringA("\n");
    FILE *f=fopen("game:\\srb2_boot.log","ab");
    if(f){fprintf(f,"FATAL: %s\r\n",b);fclose(f);}
    SRB2_DrawFatalScreen(b);
    for(;;) Sleep(1000);
}
void I_OutputMsg(const char *fmt,...){char b[2048];va_list ap;va_start(ap,fmt);vsnprintf(b,sizeof(b)-1,fmt,ap);va_end(ap);b[sizeof(b)-1]=0;OutputDebugStringA(b);}
void I_Tactile(FFType Type,const JoyFF_t *Effect){(void)Type;(void)Effect;} void I_Tactile2(FFType Type,const JoyFF_t *Effect){(void)Type;(void)Effect;}
void I_JoyScale(void){} void I_JoyScale2(void){} void I_InitJoystick(void){} void I_InitJoystick2(void){}
INT32 I_NumJoys(void){return 2;} const char *I_GetJoyName(INT32 i){return i == 1 ? "Xbox 360 Controller 2" : "Xbox 360 Controller 1";}
#ifndef NOMUMBLE
void I_UpdateMumble(const mobj_t *mobj,const listener_t listener){(void)mobj;(void)listener;}
#endif
void I_StartupMouse(void){} void I_StartupMouse2(void){} void I_StartupKeyboard(void){keyboard_started=1;} INT32 I_GetKey(void){return 0;}
void I_StartupTimer(void){s_start=GetTickCount();} void I_AddExitFunc(void (*f)()){(void)f;} void I_RemoveExitFunc(void (*f)()){(void)f;}
INT32 I_StartupSystem(void){I_StartupTimer();return 0;} void I_ShutdownSystem(void){}
void I_GetDiskFreeSpace(INT64 *f){if(f)*f=512LL*1024*1024;} char *I_GetUserName(void){static char n[]="Player";return n;}
INT32 I_mkdir(const char *d,INT32 r){(void)r;return CreateDirectoryA(d,NULL)?0:-1;}
const CPUInfoFlags *I_CPUInfo(void){return NULL;} /* IdentifyVersion appends PATHSEP itself; avoid game:\\filename on hardware. */
const char *I_LocateWad(void){return "game:";}
void I_GetJoystickEvents(void){} void I_GetJoystick2Events(void){} void I_GetMouseEvents(void){}
char *I_GetEnv(const char *n){(void)n;return NULL;} INT32 I_PutEnv(char *v){(void)v;return 0;} void I_RegisterSysCommands(void){}
