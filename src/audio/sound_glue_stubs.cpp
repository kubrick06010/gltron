#include "audio/sound_glue.h"

extern "C" {
#include "configuration/settings.h"
}

#include "libopenmpt/libopenmpt.h"
#include "SDL.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

enum SampleId {
  kSampleEngine = 0,
  kSampleCrash = 1,
  kSampleRecognizer = 2,
  kSampleMenuAction = 3,
  kSampleMenuHighlight = 4,
  kSampleCount = 5
};

struct Sample {
  Uint8 *data;
  Uint32 length;
  int allocatedWithMalloc;
};

struct Voice {
  int sample;
  Uint32 position;
  int active;
  int loop;
};

static SDL_AudioSpec gAudioSpec;
static int gAudioReady = 0;
static int gAudioStarted = 0;
static int gFxVolume = SDL_MIX_MAXVOLUME;
static int gMusicVolume = SDL_MIX_MAXVOLUME;
static int gMusicPlaying = 0;
static Sample gSamples[kSampleCount];
static Voice gEngineVoice = { kSampleEngine, 0, 0, 1 };
static Voice gRecognizerVoice = { kSampleRecognizer, 0, 0, 1 };
static Voice gCrashVoices[8];
static Voice gMenuVoices[4];
static openmpt_module *gMusicModule = NULL;

static void resetVoice(Voice *voice, int sample, int loop)
{
  voice->sample = sample;
  voice->position = 0;
  voice->active = 1;
  voice->loop = loop;
}

static void stopVoice(Voice *voice)
{
  voice->active = 0;
  voice->position = 0;
}

static void playOneShot(int sample, Voice *voices, int voiceCount)
{
  int i;

  SDL_LockAudio();
  for(i = 0; i < voiceCount; i++) {
    if(!voices[i].active) {
      resetVoice(&voices[i], sample, 0);
      SDL_UnlockAudio();
      return;
    }
  }
  resetVoice(&voices[0], sample, 0);
  SDL_UnlockAudio();
}

static void mixVoice(Uint8 *stream, int len, Voice *voice)
{
  Sample *sample;
  int remaining;
  Uint8 *dst;

  if(!voice->active || voice->sample < 0 || voice->sample >= kSampleCount) {
    return;
  }

  sample = &gSamples[voice->sample];
  if(sample->data == NULL || sample->length == 0) {
    return;
  }

  remaining = len;
  dst = stream;
  while(remaining > 0 && voice->active) {
    Uint32 available = sample->length - voice->position;
    Uint32 chunk = available < (Uint32)remaining ? available : (Uint32)remaining;

    SDL_MixAudio(dst, sample->data + voice->position, chunk, gFxVolume);
    dst += chunk;
    remaining -= (int)chunk;
    voice->position += chunk;

    if(voice->position >= sample->length) {
      if(voice->loop) {
        voice->position = 0;
      } else {
        stopVoice(voice);
      }
    }
  }
}

static void audioCallback(void *userdata, Uint8 *stream, int len)
{
  int i;
  int offset = 0;
  int16_t musicBuffer[1024 * 2];

  memset(stream, 0, len);
  while(gMusicModule != NULL && gMusicPlaying && gSettingsCache.playMusic && offset < len) {
    int remaining = len - offset;
    size_t framesRequested = (size_t)remaining / 4;
    size_t framesRead;
    size_t bytesRead;

    if(framesRequested > 1024) {
      framesRequested = 1024;
    }
    if(framesRequested == 0) {
      break;
    }

    framesRead = openmpt_module_read_interleaved_stereo(gMusicModule, gAudioSpec.freq, framesRequested, musicBuffer);
    if(framesRead == 0) {
      gMusicPlaying = 0;
      break;
    }

    bytesRead = framesRead * 4;
    SDL_MixAudio(stream + offset, (Uint8*)musicBuffer, (Uint32)bytesRead, gMusicVolume);
    offset += (int)bytesRead;
  }

  if(!gSettingsCache.playEffects) {
    return;
  }

  mixVoice(stream, len, &gEngineVoice);
  mixVoice(stream, len, &gRecognizerVoice);
  for(i = 0; i < 8; i++) {
    mixVoice(stream, len, &gCrashVoices[i]);
  }
  for(i = 0; i < 4; i++) {
    mixVoice(stream, len, &gMenuVoices[i]);
  }
}

static void freeSample(Sample *sample)
{
  if(sample->data != NULL) {
    if(sample->allocatedWithMalloc) {
      free(sample->data);
    } else {
      SDL_FreeWAV(sample->data);
    }
    sample->data = NULL;
  }
  sample->length = 0;
  sample->allocatedWithMalloc = 0;
}

static int convertSample(SDL_AudioSpec *sourceSpec, Uint8 **data, Uint32 *length, int *allocatedWithMalloc)
{
  SDL_AudioCVT cvt;

  if(SDL_BuildAudioCVT(&cvt,
                       sourceSpec->format,
                       sourceSpec->channels,
                       sourceSpec->freq,
                       gAudioSpec.format,
                       gAudioSpec.channels,
                       gAudioSpec.freq) < 0) {
    return -1;
  }

  if(!cvt.needed) {
    return 0;
  }

  cvt.len = (int)*length;
  cvt.buf = (Uint8*)malloc((size_t)cvt.len * (size_t)cvt.len_mult);
  if(cvt.buf == NULL) {
    return -1;
  }

  memcpy(cvt.buf, *data, *length);
  if(SDL_ConvertAudio(&cvt) < 0) {
    free(cvt.buf);
    return -1;
  }

  SDL_FreeWAV(*data);
  *data = cvt.buf;
  *length = (Uint32)cvt.len_cvt;
  *allocatedWithMalloc = 1;
  return 0;
}

static void unloadMusic(void)
{
  if(gMusicModule != NULL) {
    openmpt_module_destroy(gMusicModule);
    gMusicModule = NULL;
  }
  gMusicPlaying = 0;
}

}

extern "C" {

void Audio_EnableEngine(void)
{
  SDL_LockAudio();
  resetVoice(&gEngineVoice, kSampleEngine, 1);
  resetVoice(&gRecognizerVoice, kSampleRecognizer, 1);
  SDL_UnlockAudio();
}

void Audio_DisableEngine(void)
{
  SDL_LockAudio();
  stopVoice(&gEngineVoice);
  stopVoice(&gRecognizerVoice);
  SDL_UnlockAudio();
}

void Audio_Idle(void)
{
}

void Audio_CrashPlayer(int player)
{
  playOneShot(kSampleCrash, gCrashVoices, 8);
}

void Audio_Init(void)
{
  SDL_AudioSpec desired;
  int i;

  if(gAudioReady) {
    return;
  }

  if(SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
    fprintf(stderr, "[sound] SDL audio init failed: %s\n", SDL_GetError());
    return;
  }

  memset(&desired, 0, sizeof(desired));
  desired.freq = 22050;
  desired.format = AUDIO_S16SYS;
  desired.channels = 2;
  desired.samples = 1024;
  desired.callback = audioCallback;

  if(SDL_OpenAudio(&desired, &gAudioSpec) < 0) {
    fprintf(stderr, "[sound] SDL audio open failed: %s\n", SDL_GetError());
    return;
  }

  for(i = 0; i < 8; i++) {
    gCrashVoices[i].sample = kSampleCrash;
  }
  for(i = 0; i < 4; i++) {
    gMenuVoices[i].sample = kSampleMenuHighlight;
  }
  gAudioReady = 1;
}

void Audio_Start(void)
{
  if(gAudioReady && !gAudioStarted) {
    SDL_PauseAudio(0);
    gAudioStarted = 1;
  }
}

void Audio_Quit(void)
{
  int i;

  if(gAudioReady) {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    gAudioReady = 0;
    gAudioStarted = 0;
  }

  for(i = 0; i < kSampleCount; i++) {
    freeSample(&gSamples[i]);
  }
  stopVoice(&gEngineVoice);
  stopVoice(&gRecognizerVoice);
  for(i = 0; i < 8; i++) {
    stopVoice(&gCrashVoices[i]);
  }
  for(i = 0; i < 4; i++) {
    stopVoice(&gMenuVoices[i]);
  }
  unloadMusic();
}

void Audio_ResetData(void)
{
}

void Audio_LoadPlayers(void)
{
}

void Audio_UnloadPlayers(void)
{
}

void Audio_LoadSample(char *name, int number)
{
  SDL_AudioSpec sourceSpec;
  Uint8 *data = NULL;
  Uint32 length = 0;
  int allocatedWithMalloc = 0;

  if(number < 0 || number >= kSampleCount || !gAudioReady) {
    return;
  }

  if(SDL_LoadWAV(name, &sourceSpec, &data, &length) == NULL) {
    fprintf(stderr, "[sound] failed to load sample '%s': %s\n", name, SDL_GetError());
    return;
  }

  if(convertSample(&sourceSpec, &data, &length, &allocatedWithMalloc) < 0) {
    fprintf(stderr, "[sound] failed to convert sample '%s': %s\n", name, SDL_GetError());
    SDL_FreeWAV(data);
    return;
  }

  SDL_LockAudio();
  freeSample(&gSamples[number]);
  gSamples[number].data = data;
  gSamples[number].length = length;
  gSamples[number].allocatedWithMalloc = allocatedWithMalloc;
  SDL_UnlockAudio();
}

void Audio_LoadMusic(char *name)
{
  FILE *file;
  long size;
  void *data;
  int error = OPENMPT_ERROR_OK;
  const char *errorMessage = NULL;
  openmpt_module *module;

  file = fopen(name, "rb");
  if(file == NULL) {
    fprintf(stderr, "[sound] failed to open music '%s'\n", name);
    return;
  }

  if(fseek(file, 0, SEEK_END) != 0) {
    fclose(file);
    return;
  }
  size = ftell(file);
  if(size <= 0 || fseek(file, 0, SEEK_SET) != 0) {
    fclose(file);
    return;
  }

  data = malloc((size_t)size);
  if(data == NULL) {
    fclose(file);
    return;
  }
  if(fread(data, 1, (size_t)size, file) != (size_t)size) {
    fprintf(stderr, "[sound] failed to read music '%s'\n", name);
    free(data);
    fclose(file);
    return;
  }
  fclose(file);

  module = openmpt_module_create_from_memory2(data, (size_t)size, NULL, NULL, NULL, NULL, &error, &errorMessage, NULL);
  free(data);

  if(module == NULL) {
    fprintf(stderr, "[sound] failed to decode music '%s': %s\n", name, errorMessage != NULL ? errorMessage : "unknown error");
    openmpt_free_string(errorMessage);
    return;
  }
  openmpt_free_string(errorMessage);

  if(getSettingi("loopMusic")) {
    openmpt_module_set_repeat_count(module, -1);
  }

  SDL_LockAudio();
  unloadMusic();
  gMusicModule = module;
  gMusicPlaying = 0;
  SDL_UnlockAudio();
}

void Audio_PlayMusic(void)
{
  SDL_LockAudio();
  if(gMusicModule != NULL) {
    gMusicPlaying = 1;
  }
  SDL_UnlockAudio();
}

void Audio_StopMusic(void)
{
  SDL_LockAudio();
  gMusicPlaying = 0;
  SDL_UnlockAudio();
}

void Audio_SetMusicVolume(float volume)
{
  if(volume < 0) {
    volume = 0;
  } else if(volume > 1) {
    volume = 1;
  }

  SDL_LockAudio();
  gMusicVolume = (int)(volume * SDL_MIX_MAXVOLUME);
  SDL_UnlockAudio();
}

void Audio_SetFxVolume(float volume)
{
  if(volume < 0) {
    volume = 0;
  } else if(volume > 1) {
    volume = 1;
  }

  SDL_LockAudio();
  gFxVolume = (int)(volume * SDL_MIX_MAXVOLUME);
  SDL_UnlockAudio();
}

void Audio_MenuAction(void)
{
  playOneShot(kSampleMenuAction, gMenuVoices, 4);
}

void Audio_MenuHighlight(void)
{
  playOneShot(kSampleMenuHighlight, gMenuVoices, 4);
}

void Audio_StartEngine(int player)
{
  SDL_LockAudio();
  resetVoice(&gEngineVoice, kSampleEngine, 1);
  SDL_UnlockAudio();
}

void Audio_StopEngine(int player)
{
  SDL_LockAudio();
  stopVoice(&gEngineVoice);
  SDL_UnlockAudio();
}

}
