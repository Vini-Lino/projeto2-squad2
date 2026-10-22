#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <stdbool.h>

// Trilhas em assets/music/
#define MUSICA_RE7        "assets/music/Trilha_2026_Re7.mp3"
#define MUSICA_VIAGEM     "assets/music/Trilha_viagem_no_tempo.mp3"
#define MUSICA_2007       "assets/music/Trilha_2007.mp3"
#define MUSICA_AVENTURA   "assets/music/Trilha_aventura.mp3"
#define MUSICA_VITORIA    "assets/music/Trilha_vitoria_final.mp3"
#define MUSICA_RETORNO    "assets/music/Trilha_Retorno_temporal.mp3"
#define MUSICA_EPILOGO    "assets/music/Trilha_Epilogo_emocional.mp3"
#define MUSICA_FINAL      "assets/music/Trilha_final_do_jogo.mp3"

// Efeitos sonoros em assets/sounds/
typedef enum {
    SOM_MOUSE,
    SOM_DIALOGO,
    SOM_PASSADO,
    SOM_ACERTO,  // Opcional: o arquivo pode não existir
    SOM_ERRO,    // Opcional: o arquivo pode não existir
    SOM_TOTAL
} EfeitoSonoro;

void AudioManager_Init(void);
void AudioManager_Close(void);

void AudioManager_Play(EfeitoSonoro efeito);
void AudioManager_PlayMusic(const char *caminho);
void AudioManager_StopMusic(void);
void AudioManager_Update(void);

// Volumes entre 0.0f e 1.0f
void AudioManager_SetMusicVolume(float volume);
float AudioManager_GetMusicVolume(void);

void AudioManager_ToggleMusicMute(void);
bool AudioManager_IsMusicMuted(void);

void AudioManager_SetEffectsVolume(float volume);
float AudioManager_GetEffectsVolume(void);

// Silencia apenas a música ambiente; os efeitos continuam funcionando.
void AudioManager_SetMusicMuted(bool muted);
void AudioManager_ToggleMusicMute(void);
bool AudioManager_IsMusicMuted(void);

// Compatibilidade com chamadas antigas: alterna o mudo da música.
void AudioManager_ToggleMute(void);
bool AudioManager_IsMuted(void);

#endif
