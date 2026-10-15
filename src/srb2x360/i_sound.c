/* SRB2 native SFX backend. Doom-format lumps: LE16 magic=3, LE16 rate,
LE32 sample count, followed by unsigned 8-bit mono PCM. */
#include "../../srb2/src/i_sound.h"
#include "../../srb2/src/s_sound.h"
#include "../../srb2/src/console.h"
#include "../../srb2/src/w_wad.h"
#include "../../srb2/src/z_zone.h"
#include "audio_bridge.h"
#include <string.h>
UINT8 sound_started = 0;
static unsigned int rd16(const UINT8 *p) { return (unsigned int)p[0] | ((unsigned int)p[1]<<8); }
static unsigned int rd32(const UINT8 *p) { return rd16(p) | (rd16(p+2)<<16); }
void *I_GetSfx(sfxinfo_t *sfx) {
    size_t len; UINT8 *data;
    if (sfx->data) return sfx->data;
    if (sfx->lumpnum == LUMPERROR) sfx->lumpnum=S_GetSfxLumpNum(sfx);
    if (sfx->lumpnum == LUMPERROR) return NULL;
    len=W_LumpLength(sfx->lumpnum);
    if (len<9 || len>0x7fffffffU) return NULL;
    data=(UINT8*)Z_Malloc(len,PU_SOUND,NULL);
    if (!data) return NULL;
    W_ReadLump(sfx->lumpnum,data);
    if (rd16(data)!=3 || rd16(data+2)<4000 || rd16(data+2)>96000 || rd32(data+4)>len-8 || !rd32(data+4)) {
        Z_Free(data); return NULL;
    }
    sfx->length=rd32(data+4);
    return data;
}
void I_FreeSfx(sfxinfo_t *sfx) {
    if (!sfx || !sfx->data) return;
    X360_AudioStopData(((const UINT8*)sfx->data)+8);
    Z_Free(sfx->data); sfx->data=NULL; sfx->length=0; sfx->lumpnum=LUMPERROR;
}
void I_StartupSound(void) {
    sound_started=(UINT8)X360_AudioInit();
    if (!sound_started)
    CONS_Alert(CONS_ERROR, "Xbox 360 XAudio2 initialization failed; SFX disabled.\n");
    else
    CONS_Printf("Xbox 360 XAudio2 SFX initialized.\n");
}
void I_ShutdownSound(void) { X360_AudioShutdown(); sound_started=0; }
void I_UpdateSound(void) {}
INT32 I_StartSound(sfxenum_t id, UINT8 vol, UINT8 sep, UINT8 pitch, UINT8 priority) {
    const UINT8 *p; (void)priority;
    if (!sound_started || !S_sfx[id].data) return -1;
    p=(const UINT8*)S_sfx[id].data;
    return X360_AudioPlay(p+8,(unsigned int)S_sfx[id].length,rd16(p+2),vol,sep,pitch);
}
void I_StopSound(INT32 handle) { X360_AudioStop(handle); }
boolean I_SoundIsPlaying(INT32 handle) { return X360_AudioPlaying(handle) ? true : false; }
void I_UpdateSoundParams(INT32 handle, UINT8 vol, UINT8 sep, UINT8 pitch) { X360_AudioParams(handle,vol,sep,pitch); }
void I_SetSfxVolume(UINT8 volume) { X360_AudioVolume(volume); }
/* Digital music uses PC-preconverted PCM WAV sidecars in game:\music. */
void I_InitMusic(void) {}
void I_ShutdownMusic(void) { X360_MusicStop(); }
void I_PauseSong(INT32 h) { (void)h; X360_MusicPause(); }
void I_ResumeSong(INT32 h) { (void)h; X360_MusicResume(); }
void I_InitMIDIMusic(void) {}
void I_ShutdownMIDIMusic(void) {}
void I_SetMIDIMusicVolume(UINT8 v) { (void)v; }
INT32 I_RegisterSong(void *data,size_t len) { (void)data; (void)len; return -1; }
boolean I_PlaySong(INT32 h,boolean loop) { (void)h; (void)loop; return false; }
void I_StopSong(INT32 h) { (void)h; }
void I_UnRegisterSong(INT32 h) { (void)h; }
void I_InitDigMusic(void) { }
void I_ShutdownDigMusic(void) { X360_MusicStop(); }
boolean I_SetSongSpeed(float v) { return X360_MusicSpeed(v) ? true : false; }
boolean I_SetSongTrack(INT32 v) { (void)v; return true; }
boolean I_StartDigSong(const char *name,boolean loop) { return X360_MusicStart(name,loop) ? true : false; }
void I_StopDigSong(void) { X360_MusicStop(); }
void I_SetDigMusicVolume(UINT8 v) { X360_MusicVolume(v); }
