#include "../platform/stdafx.h"
#include <xaudio2.h>
#include <math.h>
#include <stdio.h>
#include "audio_bridge.h"
#define VOICES 32
#define MAX_SFX_RATIO 2.0f
struct Slot {
    IXAudio2SourceVoice *voice;
    unsigned int voiceRate;
    int active;
    const unsigned char *data;
    unsigned int serial;
    int lastVol, lastPan, lastPitch;
};
static IXAudio2 *engine = NULL;
static IXAudio2MasteringVoice *master = NULL;
static unsigned int masterChannels = 2;
static Slot slots[VOICES];
static unsigned int nextSerial = 1;
static float masterVolume = 1.0f;
static unsigned int failCount = 0, dropCount = 0;

static void logHr(const char *what, HRESULT hr) {
    char b[128];
    failCount++;
    if (failCount > 32 && (failCount & 63)) return;
    sprintf(b, "SRB2 AUDIO: %s failed hr=0x%08lX (failure #%u)\n", what, (unsigned long)hr, failCount);
    OutputDebugStringA(b);
}
static int activeCount(void) { int n=0; for (int i=0;i<VOICES;i++) if (slots[i].active) n++; return n; }

static int indexFor(int h) {
    int i = (h & 63) - 1;
    if (i < 0 || i >= VOICES || !slots[i].active || slots[i].serial != (unsigned int)(h >> 6) || !slots[i].voice) return -1;
    return i;
}
static void releaseSlot(int i) {
    if (slots[i].voice) { slots[i].voice->Stop(0); slots[i].voice->FlushSourceBuffers(); }
    slots[i].active = 0; slots[i].data = NULL; slots[i].serial = 0;
}
static void destroySlot(int i) {
    if (slots[i].voice) { slots[i].voice->Stop(0); slots[i].voice->FlushSourceBuffers(); slots[i].voice->DestroyVoice(); }
    slots[i].voice = NULL; slots[i].voiceRate = 0; slots[i].active = 0;
    slots[i].data = NULL; slots[i].serial = 0;
}
static void parameters(int i, unsigned char vol, unsigned char pan, unsigned char pitch, int force) {
    if (!force && slots[i].lastVol == vol && slots[i].lastPan == pan && slots[i].lastPitch == pitch) return;
    HRESULT hr;
    if (force || slots[i].lastVol != vol) {
        hr = slots[i].voice->SetVolume((float)vol / 255.0f * masterVolume);
        if (FAILED(hr)) logHr("SetVolume", hr);
    }
    if (force || slots[i].lastPan != pan) {
        float left = pan <= 128 ? 1.0f : (255.0f-pan)/127.0f;
        float right = pan >= 128 ? 1.0f : (float)pan/128.0f;
        float matrix[8]; for (int k=0;k<8;k++) matrix[k]=0.0f;
        matrix[0]=left;
        if (masterChannels >= 2) matrix[1]=right; else matrix[0]=(left+right)*0.5f;
            hr = slots[i].voice->SetOutputMatrix(NULL, 1, masterChannels > 8 ? 8 : masterChannels, matrix);
        if (FAILED(hr)) logHr("SetOutputMatrix", hr);
    }
    if (force || slots[i].lastPitch != pitch) {
        float ratio = (float)pitch / 128.0f;
        if (ratio < XAUDIO2_MIN_FREQ_RATIO) ratio = XAUDIO2_MIN_FREQ_RATIO;
        if (ratio > MAX_SFX_RATIO) ratio = MAX_SFX_RATIO;
        hr = slots[i].voice->SetFrequencyRatio(ratio);
        if (FAILED(hr)) logHr("SetFrequencyRatio", hr);
    }
    slots[i].lastVol = vol; slots[i].lastPan = pan; slots[i].lastPitch = pitch;
}
extern "C" int X360_AudioInit(void) {
    if (engine) return 1;
    HRESULT hr = XAudio2Create(&engine, 0, XAUDIO2_DEFAULT_PROCESSOR);
    if (FAILED(hr) || !engine) { logHr("XAudio2Create", hr); engine = NULL; return 0; }
    hr = engine->CreateMasteringVoice(&master);
    if (FAILED(hr) || !master) { logHr("CreateMasteringVoice", hr); engine->Release(); engine = NULL; return 0; }
    XAUDIO2_VOICE_DETAILS details; ZeroMemory(&details, sizeof(details));
    master->GetVoiceDetails(&details);
    masterChannels = details.InputChannels ? details.InputChannels : 2;
    {
        char b[96]; sprintf(b, "SRB2 AUDIO: mastering voice has %u channels\n", masterChannels);
        OutputDebugStringA(b);
    }
    ZeroMemory(slots, sizeof(slots));
    return 1;
}
extern "C" void X360_AudioShutdown(void) {
    X360_MusicShutdown();
    for (int i=0;i<VOICES;i++) destroySlot(i);
    if (master) { master->DestroyVoice(); master=NULL; }
    if (engine) { engine->Release(); engine=NULL; }
}
extern "C" int X360_AudioPlay(const unsigned char *pcm, unsigned int bytes, unsigned int rate, unsigned char volume, unsigned char pan, unsigned char pitch) {
    if (!engine || !pcm || !bytes || rate < 4000 || rate > 96000) return -1;
    int chosen=-1;
    for (int i=0;i<VOICES;i++) {
        if (!slots[i].active) continue;
        XAUDIO2_VOICE_STATE state; slots[i].voice->GetState(&state);
        if (!state.BuffersQueued) releaseSlot(i);
    }
    for (int i=0;i<VOICES && chosen<0;i++)
    if (!slots[i].active && slots[i].voice && slots[i].voiceRate == rate) chosen=i;
    for (int i=0;i<VOICES && chosen<0;i++)
    if (!slots[i].voice) chosen=i;
    for (int i=0;i<VOICES && chosen<0;i++)
    if (!slots[i].active) { destroySlot(i); chosen=i; }
    if (chosen<0) {
        dropCount++;
        if (dropCount==1 || (dropCount & 31)==0) {
            char b[96]; sprintf(b, "SRB2 AUDIO: all %d voices busy, sound dropped (#%u)\n", VOICES, dropCount);
            OutputDebugStringA(b);
        }
        return -1;
    }
    if (!slots[chosen].voice) {
        WAVEFORMATEX format; ZeroMemory(&format,sizeof(format));
        format.wFormatTag=WAVE_FORMAT_PCM; format.nChannels=1; format.nSamplesPerSec=rate;
        format.wBitsPerSample=8; format.nBlockAlign=1; format.nAvgBytesPerSec=rate;
        HRESULT hr=engine->CreateSourceVoice(&slots[chosen].voice,&format,0,MAX_SFX_RATIO);
        if (FAILED(hr) || !slots[chosen].voice) { logHr("CreateSourceVoice", hr); slots[chosen].voice=NULL; return -1; }
        slots[chosen].voiceRate=rate;
    }
    unsigned int serial=nextSerial++ & 0x01ffffff;
    if (!serial) serial=nextSerial++ & 0x01ffffff;
    slots[chosen].serial=serial; slots[chosen].data=pcm; slots[chosen].active=1;
    XAUDIO2_BUFFER buf; ZeroMemory(&buf,sizeof(buf)); buf.AudioBytes=bytes; buf.pAudioData=pcm; buf.Flags=XAUDIO2_END_OF_STREAM;
    HRESULT hr=slots[chosen].voice->SubmitSourceBuffer(&buf);
    if (FAILED(hr)) { logHr("SubmitSourceBuffer", hr); destroySlot(chosen); return -1; }
    parameters(chosen,volume,pan,pitch,1);
    hr=slots[chosen].voice->Start(0);
    if (FAILED(hr)) { logHr("Start", hr); destroySlot(chosen); return -1; }
    return (int)((serial<<6) | (chosen+1));
}
extern "C" void X360_AudioStop(int handle) { int i=indexFor(handle); if (i>=0) releaseSlot(i); }
extern "C" int X360_AudioPlaying(int handle) {
    int i=indexFor(handle); if(i<0) return 0;
    XAUDIO2_VOICE_STATE state; slots[i].voice->GetState(&state);
    if (!state.BuffersQueued) { releaseSlot(i); return 0; }
    return 1;
}
extern "C" void X360_AudioParams(int handle,unsigned char vol,unsigned char pan,unsigned char pitch) {
    int i=indexFor(handle); if(i>=0) parameters(i,vol,pan,pitch,0);
}
extern "C" void X360_AudioStopData(const unsigned char *data) {
    for(int i=0;i<VOICES;i++) if(slots[i].voice && slots[i].data==data) destroySlot(i);
}
extern "C" void X360_AudioVolume(unsigned char volume) {
    masterVolume=(float)volume/31.0f; if(masterVolume>1.0f) masterVolume=1.0f;
    for (int i=0;i<VOICES;i++) if (slots[i].active && slots[i].lastVol>=0) {
        slots[i].voice->SetVolume((float)slots[i].lastVol/255.0f*masterVolume);
    }
}

extern "C" IXAudio2 *X360_GetAudioEngine(void) { return engine; }
