#ifndef SRB2_X360_AUDIO_BRIDGE_H
#define SRB2_X360_AUDIO_BRIDGE_H
#ifdef __cplusplus
extern "C" {
#endif
    int X360_AudioInit(void);
    void X360_AudioShutdown(void);
    int X360_AudioPlay(const unsigned char *pcm, unsigned int bytes, unsigned int rate, unsigned char volume, unsigned char pan, unsigned char pitch);
    void X360_AudioStop(int handle);
    int X360_AudioPlaying(int handle);
    void X360_AudioParams(int handle, unsigned char volume, unsigned char pan, unsigned char pitch);
    void X360_AudioStopData(const unsigned char *data);
    void X360_AudioVolume(unsigned char volume);
    int X360_MusicStart(const char *name, int looping);
    void X360_MusicStop(void);
    void X360_MusicShutdown(void); /* stop music AND end the streaming worker thread */
    void X360_MusicPause(void);
    void X360_MusicResume(void);
    void X360_MusicVolume(unsigned char volume);
    int X360_MusicSpeed(float speed);
#ifdef __cplusplus
}
#endif
#endif
