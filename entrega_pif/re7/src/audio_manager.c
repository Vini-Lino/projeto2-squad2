#include "audio_manager.h"
#include "raylib.h"
#include <string.h>

static Sound sons[SOM_TOTAL] = {0};
static bool carregado[SOM_TOTAL] = {0};
static bool inicializado = false;

static float volumeMusica = 0.5f;
static float volumeEfeitos = 0.8f;
static bool musicaMutada = false;

static const char *caminhos[SOM_TOTAL] = {
    [SOM_MOUSE]   = "assets/sounds/mouse.wav",
    [SOM_DIALOGO] = "assets/sounds/dialogo.wav",
    [SOM_PASSADO] = "assets/sounds/passado.wav",
    [SOM_ACERTO]  = "assets/sounds/acerto.wav",
    [SOM_ERRO]    = "assets/sounds/erro.wav"
};

static Music musica = {0};
static bool musicaCarregada = false;
static char musicaAtual[256] = "";

static float LimitarVolume(float volume)
{
    if (volume < 0.0f) return 0.0f;
    if (volume > 1.0f) return 1.0f;
    return volume;
}

static void AplicarVolumeMusica(void)
{
    if (musicaCarregada)
        SetMusicVolume(musica, musicaMutada ? 0.0f : volumeMusica);
}

void AudioManager_Init(void)
{
    if (inicializado) return;

    InitAudioDevice();
    if (!IsAudioDeviceReady()) {
        TraceLog(LOG_WARNING, "Nao foi possivel iniciar o audio");
        return;
    }

    inicializado = true;

    for (int i = 0; i < SOM_TOTAL; i++) {
        if (!FileExists(caminhos[i])) {
            TraceLog(LOG_WARNING, "Som nao encontrado: %s", caminhos[i]);
            continue;
        }

        sons[i] = LoadSound(caminhos[i]);
        carregado[i] = sons[i].frameCount > 0;
        if (carregado[i])
            SetSoundVolume(sons[i], volumeEfeitos);
        else
            TraceLog(LOG_WARNING, "Falha ao carregar: %s", caminhos[i]);
    }
}

void AudioManager_Play(EfeitoSonoro efeito)
{
    if (!inicializado || efeito < 0 || efeito >= SOM_TOTAL) return;
    if (carregado[efeito]) {
        SetSoundVolume(sons[efeito], volumeEfeitos);
        PlaySound(sons[efeito]);
    }
}

void AudioManager_StopMusic(void)
{
    if (musicaCarregada) {
        StopMusicStream(musica);
        UnloadMusicStream(musica);
        musica = (Music){0};
        musicaCarregada = false;
    }
    musicaAtual[0] = '\0';
}

void AudioManager_PlayMusic(const char *caminho)
{
    if (!inicializado || caminho == NULL) return;
    if (strcmp(musicaAtual, caminho) == 0) return;

    AudioManager_StopMusic();

    strncpy(musicaAtual, caminho, sizeof(musicaAtual) - 1);
    musicaAtual[sizeof(musicaAtual) - 1] = '\0';

    if (!FileExists(caminho)) {
        TraceLog(LOG_WARNING, "Musica nao encontrada: %s", caminho);
        return;
    }

    musica = LoadMusicStream(caminho);
    if (musica.frameCount == 0) {
        TraceLog(LOG_WARNING, "Falha ao carregar musica: %s", caminho);
        return;
    }

    musica.looping = true;
    musicaCarregada = true;
    AplicarVolumeMusica();
    PlayMusicStream(musica);
}

void AudioManager_Update(void)
{
    if (inicializado && musicaCarregada)
        UpdateMusicStream(musica);
}

void AudioManager_SetMusicVolume(float volume)
{
    volumeMusica = LimitarVolume(volume);
    AplicarVolumeMusica();
}

float AudioManager_GetMusicVolume(void)
{
    return volumeMusica;
}

void AudioManager_SetEffectsVolume(float volume)
{
    volumeEfeitos = LimitarVolume(volume);
    for (int i = 0; i < SOM_TOTAL; i++)
        if (carregado[i]) SetSoundVolume(sons[i], volumeEfeitos);
}

float AudioManager_GetEffectsVolume(void)
{
    return volumeEfeitos;
}

void AudioManager_SetMusicMuted(bool muted)
{
    musicaMutada = muted;
    AplicarVolumeMusica();
}

void AudioManager_ToggleMusicMute(void)
{
    AudioManager_SetMusicMuted(!musicaMutada);
}

bool AudioManager_IsMusicMuted(void)
{
    return musicaMutada;
}

void AudioManager_ToggleMute(void)
{
    AudioManager_ToggleMusicMute();
}

bool AudioManager_IsMuted(void)
{
    return AudioManager_IsMusicMuted();
}

void AudioManager_Close(void)
{
    if (!inicializado) return;

    AudioManager_StopMusic();
    for (int i = 0; i < SOM_TOTAL; i++) {
        if (carregado[i]) {
            UnloadSound(sons[i]);
            sons[i] = (Sound){0};
            carregado[i] = false;
        }
    }

    CloseAudioDevice();
    inicializado = false;
}
