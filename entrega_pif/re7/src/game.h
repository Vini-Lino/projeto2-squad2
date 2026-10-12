#ifndef GAME_H
#define GAME_H

#include "raylib.h"

#include "assets.h"
#include "scenes.h"


// Qual tela do jogo está aberta?

typedef enum
{
    SCREEN_MENU,
    SCREEN_GAME,
    SCREEN_CREDITS,
    SCREEN_MINIGAME,
    SCREEN_ENDING
} GameScreen;


// Tudo que precisamos saber
// sobre o estado atual do jogo.

typedef struct
{
    GameScreen currentScreen;

    SceneID currentScene;

    int dialogueIndex;

    bool shouldExit;

} GameState;


// Funções principais

void InitGame(GameState *game);

void UpdateGame(GameState *game);

void DrawGame(
    GameState *game,
    Assets *assets
);

void ChangeScene(
    GameState *game,
    SceneID nextScene
);

#endif