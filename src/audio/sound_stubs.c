#include "audio/audio.h"
#include "audio/sound_glue.h"
#include "configuration/settings.h"
#include "filesystem/path.h"
#include "game/gltron.h"
#include "Nebu_filesystem.h"
#include "scripting/nebu_scripting.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_GAME_FX 5

static char *game_fx_names[] = {
  "game_engine.wav",
  "game_crash.wav",
  "game_recognizer.wav",
  "menu_action.wav",
  "menu_highlight.wav"
};

static char *getSoundPath(const char *filename) {
  const char *data_dir = getDirectory(PATH_DATA);
  size_t data_len = strlen(data_dir);
  size_t root_len = data_len;
  char *path;

  if(data_len >= 5 && strcmp(data_dir + data_len - 5, "/data") == 0) {
    root_len = data_len - 5;
  }

  path = malloc(root_len + strlen("/sounds/") + strlen(filename) + 1);
  if(path == NULL) {
    return NULL;
  }

  memcpy(path, data_dir, root_len);
  sprintf(path + root_len, "/sounds/%s", filename);
  return path;
}

static int isMusicModule(const char *filename) {
  size_t len = strlen(filename);

  return len > 3 && strcmp(filename + len - 3, ".it") == 0;
}

void Sound_loadFX(void) {
  int i;

  for(i = 0; i < NUM_GAME_FX; i++) {
    char *path = i < 3 ? getPath(PATH_DATA, game_fx_names[i]) : getSoundPath(game_fx_names[i]);
    if(path != NULL) {
      Audio_LoadSample(path, i);
      free(path);
    }
  }
}

void Sound_reloadTrack(void) {
  char *song;
  char *path;

  if(!getSettingi("playMusic")) {
    Sound_stop();
    return;
  }

  scripting_GetGlobal("settings", "current_track", NULL);
  scripting_GetStringResult(&song);
  path = getPath(PATH_MUSIC, song);
  scripting_StringResult_Free(song);

  if(path != NULL) {
    Sound_load(path);
    Sound_play();
    free(path);
  }
}

void Sound_shutdown(void) {
  Audio_UnloadPlayers();
  Audio_Quit();
}

void Sound_load(char *name) {
  Audio_LoadMusic(name);
}

void Sound_play(void) {
  Audio_PlayMusic();
}

void Sound_stop(void) {
  Audio_StopMusic();
}

void Sound_idle(void) {
  Audio_Idle();
}

void Sound_setMusicVolume(float volume) {
  Audio_SetMusicVolume(volume);
}

void Sound_setFxVolume(float volume) {
  if(volume > 1) volume = 1;
  if(volume < 0) volume = 0;
  Audio_SetFxVolume(volume);
}

void Sound_initTracks(void) {
  const char *music_path;
  nebu_List *soundList;
  nebu_List *p;
  int i;

  music_path = getDirectory(PATH_MUSIC);
  soundList = readDirectoryContents(music_path, NULL);
  if(soundList == NULL || soundList->next == NULL) {
    fprintf(stderr, "[sound] no music files found; using silent fallback track\n");
    scripting_Run("tracks[1] = \"\"");
    scripting_Run("setupSoundTrack()");
    if(soundList != NULL) {
      nebu_List_Free(soundList);
    }
    return;
  }

  i = 1;
  for(p = soundList; p->next != NULL; p = p->next) {
    char *path = getPossiblePath(PATH_MUSIC, (char*)p->data);
    if(path != NULL && isMusicModule((char*)p->data) && nebu_FS_Test(path)) {
      scripting_RunFormat("tracks[%d] = \"%s\"", i, (char*)p->data);
      i++;
    }
    free(path);
    free(p->data);
  }
  nebu_List_Free(soundList);

  if(i == 1) {
    scripting_Run("tracks[1] = \"\"");
  }
  scripting_Run("setupSoundTrack()");
}

void Sound_setup(void) {
  Audio_Init();
  Sound_loadFX();
  Sound_setFxVolume(getSettingf("fxVolume"));
  Sound_reloadTrack();
  Sound_setMusicVolume(getSettingf("musicVolume"));
  Audio_Start();
}
