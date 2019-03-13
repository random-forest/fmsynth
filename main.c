#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void playAudio (voice *v) {
  float sample;
  Uint32 sourceIndex;
  double phaseIncrement = v->frequency/sampleRate;
  Uint32 i;

  if (v->volume > practicallySilent) {
    for (i = 0; (i + 1) < samplesPerFrame; i += 2) {
      v->phase += phaseIncrement;
        if (v->phase > 1) v->phase -= 1;

        sourceIndex = v->phase*v->waveformLength;
        sample = v->waveform[sourceIndex]*v->volume;

        audioBuffer[audioMainLeftOff+i] += sample*(1-v->pan);
        audioBuffer[audioMainLeftOff+i+1] += sample*v->pan;
    }
  } else {
    for (i=0; i<samplesPerFrame; i+=1)
      audioBuffer[audioMainLeftOff+i] = 0;
  }

  audioMainAccumulator++;
}

double getFrequency(double pitch) {
  return pow(ChromaticRatio, pitch - 57) * 440;
}

int getWaveformLength(double pitch) {
  return sampleRate / getFrequency(pitch) + 0.5f;
}

void buildSineWave(float *data, Uint32 length) {
  Uint32 i;
  for (i=0; i < length; i++)
    data[i] = sin(i * (Tao / length));
}

void logSpec(SDL_AudioSpec *as) {
  printf(
    " freq______%5d\n"
    " format____%5d\n"
    " channels__%5d\n"
    " silence___%5d\n"
    " samples___%5d\n"
    " size______%5d\n\n",
    (int) as->freq,
    (int) as->format,
    (int) as->channels,
    (int) as->silence,
    (int) as->samples,
    (int) as->size
  );
}

void logVoice(voice *v) {
  printf(
    " waveformLength__%d\n"
    " volume__________%f\n"
    " pan_____________%f\n"
    " frequency_______%f\n"
    " phase___________%f\n",
    v->waveformLength,
    v->volume,
    v->pan,
    v->frequency,
    v->phase
  );
}

void logWavedata(float *floatStream, Uint32 floatStreamLength, Uint32 increment) {
  printf("\n\nwaveform data:\n\n");
  Uint32 i=0;

  for (i = 0; i < floatStreamLength; i += increment)
    printf("%4d:%2.16f\n", i, floatStream[i]);

  printf("\n\n");
}

void audioCallback(void *unused, Uint8 *byteStream, int byteStreamLength) {
  float* floatStream = (float*) byteStream;
  Sint32 localAudioCallbackLeftOff = SDL_AtomicGet(&audioCallbackLeftOff);
  Uint32 i;
  for (i = 0; i < floatStreamLength; i++) {
    floatStream[i] = audioBuffer[localAudioCallbackLeftOff];
    localAudioCallbackLeftOff++;
    if (localAudioCallbackLeftOff == audioBufferLength)
      localAudioCallbackLeftOff = 0;
  }

  SDL_AtomicSet(&audioCallbackLeftOff, localAudioCallbackLeftOff);
}

int init(void) {
  SDL_AudioSpec want;

  SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER);
  SDL_zero(want);

  SDL_Window * window = SDL_CreateWindow(WINDOW_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
  SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, 0);

  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

  want.freq = sampleRate;
  want.format = AUDIO_F32;
  want.channels = 2;
  want.samples = floatStreamLength;
  want.callback = audioCallback;

  AudioDevice = SDL_OpenAudioDevice(NULL, 0, &want, &audioSpec, SDL_AUDIO_ALLOW_FORMAT_CHANGE);

  if (AudioDevice == 0) {
    printf("\nFailed to open audio: %s\n", SDL_GetError());
    return 1;
  }

    // printf("want:\n");
    // logSpec(&want);
    // printf("audioSpec:\n");
    // logSpec(&audioSpec);

  if (audioSpec.format != want.format) {
    printf("\nCouldn't get Float32 audio format.\n");
    return 2;
  }

  sampleRate = audioSpec.freq;
  floatStreamLength = audioSpec.size / 4;
  samplesPerFrame = sampleRate / frameRate;
  msPerFrame = 1000 / frameRate;
  audioMainLeftOff = samplesPerFrame * 8;

  SDL_AtomicSet(&audioCallbackLeftOff, 0);

  if (audioBufferLength % samplesPerFrame)
    audioBufferLength += samplesPerFrame - (audioBufferLength % samplesPerFrame);

  audioBuffer = malloc(sizeof(float) * audioBufferLength);

  return 0;
}

int onExit(void) {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_CloseAudioDevice(AudioDevice);
  SDL_Quit();
  free(audioBuffer);

  return 0;
}

int main(int argc, char *argv[]) {
  float  syncCompensationFactor = 0.0016;

  double freq0 = 45;
  double freq1 = 49;
  double freq2 = 52;

  double *frq0 = &freq0;
  double *frq1 = &freq1;
  double *frq2 = &freq2;

  Sint32 mainAudioLead;
  Uint32 i;

  voice testVoiceA;
  voice testVoiceB;
  voice testVoiceC;
  
  testVoiceA.volume = 1;
  testVoiceB.volume = 1;
  testVoiceC.volume = 1;

  testVoiceA.pan = 0.5;
  testVoiceB.pan = 0;
  testVoiceC.pan = 1;

  testVoiceA.phase = 0;
  testVoiceB.phase = 0;
  testVoiceC.phase = 0;

  testVoiceA.frequency = getFrequency(freq0);
  testVoiceB.frequency = getFrequency(freq1);
  testVoiceC.frequency = getFrequency(freq2);

  Uint16 C0waveformLength = getWaveformLength(0);

  testVoiceA.waveformLength = C0waveformLength;
  testVoiceB.waveformLength = C0waveformLength;
  testVoiceC.waveformLength = C0waveformLength;

  float sineWave[C0waveformLength];
  buildSineWave(sineWave, C0waveformLength);

  testVoiceA.waveform = sineWave;
  testVoiceB.waveform = sineWave;
  testVoiceC.waveform = sineWave;

  if (init()) return 1;

  SDL_Delay(42);
  SDL_PauseAudioDevice(AudioDevice, 0);

  while (running) {
    while (SDL_PollEvent(&event) != 0) {
      switch (event.type) {
        case SDL_QUIT:
          running = SDL_FALSE;
        break;
        case SDL_KEYDOWN:
          switch (event.key.keysym.sym) {
            case SDLK_LEFT:
              testVoiceA.frequency = getFrequency(freq0);
              *frq0 -= 0.1;
            break;
            case SDLK_RIGHT:
              testVoiceA.frequency = getFrequency(freq0);
              *frq0 += 0.1;
            break;
            case SDLK_UP:
            break;
            case SDLK_DOWN:
            break;
          }
        break;
      }
    }
        
    for (i = 0; i < samplesPerFrame; i++)
      audioBuffer[audioMainLeftOff+i] = 0;

    playAudio(&testVoiceA);
    playAudio(&testVoiceB);
    playAudio(&testVoiceC);

    if (audioMainAccumulator > 1) {
      for (i = 0; i < samplesPerFrame; i++) {
        audioBuffer[audioMainLeftOff+i] /= audioMainAccumulator;
      }
    }

    audioMainAccumulator = 0;
    audioMainLeftOff += samplesPerFrame;

    if (audioMainLeftOff == audioBufferLength) audioMainLeftOff = 0;

    mainAudioLead = audioMainLeftOff - SDL_AtomicGet(&audioCallbackLeftOff);

    if (mainAudioLead < 0)
      mainAudioLead += audioBufferLength;

    if (mainAudioLead < floatStreamLength)
      printf("An audio collision may have occured!\n");

    SDL_Delay(mainAudioLead * syncCompensationFactor);
  }

  onExit();

  return 0;
}