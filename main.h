#include <SDL.h>

#define WINDOW_TITLE "))))))))))(((())("

const double ChromaticRatio = 1.059463094359295264562;
const double Tao = 6.283185307179586476925; 
double practicallySilent = 0.001;
float *audioBuffer;

Uint32 sampleRate = 48000;
Uint32 frameRate = 60;
Uint32 floatStreamLength = 1024;
Uint32 audioBufferLength = 48000;
Uint32 samplesPerFrame;
Sint32 audioMainLeftOff;
Uint32 msPerFrame;
Uint8 audioMainAccumulator;

SDL_atomic_t audioCallbackLeftOff;
SDL_AudioDeviceID AudioDevice;
SDL_AudioSpec audioSpec;

SDL_Window * window;
SDL_Renderer * renderer;

SDL_Event event;
SDL_bool running = SDL_TRUE;

typedef struct {
    float *waveform;
    Uint32 waveformLength;
    double volume;
    double pan;
    double frequency;
    double phase;
} voice;