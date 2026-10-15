#include "../platform/stdafx.h"
#include <xaudio2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "audio_bridge.h"
extern "C" IXAudio2 *X360_GetAudioEngine(void);

#define NBUF   4
#define CHUNK  32768
#define WORKER_HW_THREAD 4

static CRITICAL_SECTION cs;
static int csInit = 0;
static HANDLE worker = NULL;
static volatile LONG quitWorker = 0;

static IXAudio2SourceVoice *musicVoice = NULL;
static FILE *musicFile = NULL;
static unsigned char chunkBuf[NBUF][CHUNK];
static unsigned int nextBuf = 0;
static unsigned int dataOffset = 0, dataLength = 0, dataPos = 0;
static int musicLooping = 0, musicActive = 0, musicEOS = 0;
static float musicVolume = 18.0f / 31.0f;
static float musicSpeed = 1.0f;
static int musicPaused = 0;

static unsigned int read16(const unsigned char *p) { return p[0] | ((unsigned int)p[1]<<8); }
static unsigned int read32(const unsigned char *p) { return read16(p) | (read16(p+2)<<16); }

static void musicLog(const char *a, const char *b) {
    OutputDebugStringA(a); if (b) OutputDebugStringA(b); OutputDebugStringA("\n");
}

static void stopLocked(void) {
    musicActive = 0; musicEOS = 0;
    if (musicVoice) { musicVoice->Stop(0); musicVoice->FlushSourceBuffers(); musicVoice->DestroyVoice(); musicVoice = NULL; }
    if (musicFile) { fclose(musicFile); musicFile = NULL; }
    musicPaused = 0; musicSpeed = 1.0f; nextBuf = 0; dataPos = 0;
}

static void pumpLocked(int maxSubmit) {
    while (musicActive && !musicEOS && musicVoice && maxSubmit > 0) {
        XAUDIO2_VOICE_STATE st; musicVoice->GetState(&st);
        if (st.BuffersQueued >= NBUF) return;
        if (dataPos >= dataLength) {
            if (!musicLooping) { musicEOS = 1; return; }
            if (fseek(musicFile, (long)dataOffset, SEEK_SET) != 0) { musicLog("SRB2 MUSIC loop seek failure", NULL); stopLocked(); return; }
            dataPos = 0;
        }
        unsigned int want = dataLength - dataPos; if (want > CHUNK) want = CHUNK;
        unsigned int got = (unsigned int)fread(chunkBuf[nextBuf], 1, want, musicFile);
        got &= ~1u;
        if (got == 0) {
            if (dataPos == 0) { musicLog("SRB2 MUSIC read failure", NULL); stopLocked(); return; }
            dataPos = dataLength;
            continue;
        }
        for (unsigned int i = 0; i < got; i += 2) { unsigned char c = chunkBuf[nextBuf][i]; chunkBuf[nextBuf][i] = chunkBuf[nextBuf][i+1]; chunkBuf[nextBuf][i+1] = c; }
        XAUDIO2_BUFFER buf; ZeroMemory(&buf, sizeof(buf));
        buf.AudioBytes = got; buf.pAudioData = chunkBuf[nextBuf];
        if (!musicLooping && dataPos + got >= dataLength) buf.Flags = XAUDIO2_END_OF_STREAM;
        HRESULT hr = musicVoice->SubmitSourceBuffer(&buf);
        if (FAILED(hr)) { char b[64]; sprintf(b, "SRB2 MUSIC SubmitSourceBuffer hr=0x%08lX", (unsigned long)hr); musicLog(b, NULL); stopLocked(); return; }
        dataPos += got; nextBuf = (nextBuf + 1) % NBUF; maxSubmit--;
    }
}

static DWORD WINAPI workerMain(LPVOID) {
    while (!quitWorker) {
        Sleep(20);
        EnterCriticalSection(&cs);
        pumpLocked(NBUF);
        LeaveCriticalSection(&cs);
    }
    return 0;
}

static int ensureWorker(void) {
    if (!csInit) { InitializeCriticalSection(&cs); csInit = 1; }
    if (worker) return 1;
    quitWorker = 0;
    DWORD id = 0;
    worker = CreateThread(NULL, 0, workerMain, NULL, CREATE_SUSPENDED, &id);
    if (!worker) { musicLog("SRB2 MUSIC: CreateThread failed", NULL); return 0; }
#ifdef _XBOX
    XSetThreadProcessor(worker, WORKER_HW_THREAD);
#endif
    ResumeThread(worker);
    return 1;
}

extern "C" void X360_MusicStop(void) {
    if (!csInit) return;
    EnterCriticalSection(&cs); stopLocked(); LeaveCriticalSection(&cs);
}
extern "C" void X360_MusicShutdown(void) {
    X360_MusicStop();
    if (worker) {
        quitWorker = 1;
        WaitForSingleObject(worker, INFINITE);
        CloseHandle(worker); worker = NULL;
    }
    if (csInit) { DeleteCriticalSection(&cs); csInit = 0; }
}
extern "C" void X360_MusicPause(void) {
    if (!csInit) return;
    EnterCriticalSection(&cs);
    if (musicVoice && !musicPaused) { musicVoice->Stop(0); musicPaused = 1; }
    LeaveCriticalSection(&cs);
}
extern "C" void X360_MusicResume(void) {
    if (!csInit) return;
    EnterCriticalSection(&cs);
    if (musicVoice && musicPaused) { musicVoice->Start(0); musicPaused = 0; }
    LeaveCriticalSection(&cs);
}
extern "C" void X360_MusicVolume(unsigned char v) {
    musicVolume = (float)(v > 31 ? 31 : v) / 31.0f;
    if (!csInit) return;
    EnterCriticalSection(&cs);
    if (musicVoice) musicVoice->SetVolume(musicVolume);
    LeaveCriticalSection(&cs);
}
extern "C" int X360_MusicSpeed(float speed) {
    if (speed < 0.5f || speed > 2.0f) return 0;
    if (!csInit) { musicSpeed = speed; return 1; }
    EnterCriticalSection(&cs);
    musicSpeed = speed;
    if (musicVoice) musicVoice->SetFrequencyRatio(speed);
    LeaveCriticalSection(&cs);
    return 1;
}

extern "C" int X360_MusicStart(const char *name, int looping) {
    char path[64]; unsigned char header[12], chunk[8], fmt[32];
    FILE *f = NULL; long fileEnd; unsigned int length = 0, rate = 0, offset = 0; int validFmt = 0;
    IXAudio2 *engine = X360_GetAudioEngine(); HRESULT hr; WAVEFORMATEX wf;
    X360_MusicStop();
    if (!engine || !name || !*name || strlen(name) > 8 || strspn(name, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") != strlen(name)) return 0;
    if (!ensureWorker()) return 0;
    sprintf(path, "game:\\music\\%s.wav", name);
    f = fopen(path, "rb");
    if (!f) { musicLog("SRB2 MUSIC missing: ", path); return 0; }
    if (fseek(f, 0, SEEK_END) != 0 || (fileEnd = ftell(f)) < 44 || fseek(f, 0, SEEK_SET) != 0) goto bad;
    if (fread(header, 1, 12, f) != 12 || memcmp(header, "RIFF", 4) || memcmp(header + 8, "WAVE", 4)) goto bad;
    while (ftell(f) >= 0 && ftell(f) + 8 <= fileEnd) {
        long here; unsigned int size;
        if (fread(chunk, 1, 8, f) != 8) goto bad;
        here = ftell(f); size = read32(chunk + 4);
        if ((unsigned long)size > (unsigned long)(fileEnd - here)) goto bad;
        if (!memcmp(chunk, "fmt ", 4)) {
            if (size < 16 || size > sizeof(fmt) || fread(fmt, 1, size, f) != size) goto bad;
            if (read16(fmt) != 1 || read16(fmt + 2) != 1 || read16(fmt + 14) != 16 || read16(fmt + 12) != 2) goto bad;
            rate = read32(fmt + 4); if (rate < 8000 || rate > 48000 || read32(fmt + 8) != rate * 2) goto bad;
            validFmt = 1;
        } else if (!memcmp(chunk, "data", 4)) { offset = (unsigned int)here; length = size; }
        if (fseek(f, here + size + (size & 1), SEEK_SET) != 0) goto bad;
    }
    length &= ~1u;
    if (!validFmt || !length || !offset) goto bad;
    if (fseek(f, (long)offset, SEEK_SET) != 0) goto bad;

    ZeroMemory(&wf, sizeof(wf)); wf.wFormatTag = WAVE_FORMAT_PCM; wf.nChannels = 1;
    wf.nSamplesPerSec = rate; wf.wBitsPerSample = 16; wf.nBlockAlign = 2; wf.nAvgBytesPerSec = rate * 2;

    EnterCriticalSection(&cs);
    hr = engine->CreateSourceVoice(&musicVoice, &wf, 0, XAUDIO2_DEFAULT_FREQ_RATIO);
    if (FAILED(hr) || !musicVoice) {
        char b[64]; sprintf(b, "SRB2 MUSIC CreateSourceVoice hr=0x%08lX", (unsigned long)hr); musicLog(b, NULL);
        musicVoice = NULL; LeaveCriticalSection(&cs); goto bad;
    }
    musicFile = f; f = NULL;
    dataOffset = offset; dataLength = length; dataPos = 0; nextBuf = 0;
    musicLooping = looping ? 1 : 0; musicEOS = 0; musicPaused = 0; musicSpeed = 1.0f;
    musicActive = 1;
    musicVoice->SetVolume(musicVolume);
    pumpLocked(1);
    if (musicActive && musicVoice) {
        if (FAILED(musicVoice->Start(0))) { musicLog("SRB2 MUSIC Start failed", NULL); stopLocked(); LeaveCriticalSection(&cs); return 0; }
        musicLog("SRB2 MUSIC streaming: ", path);
    }
    LeaveCriticalSection(&cs);
    return musicActive ? 1 : 0;
    bad:
    if (f) fclose(f);
    X360_MusicStop();
    musicLog("SRB2 MUSIC invalid WAV / playback failure", NULL);
    return 0;
}
