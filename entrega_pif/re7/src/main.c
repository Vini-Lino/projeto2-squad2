#include "raylib.h"

#include "game.h"
#include "assets.h"
#include "audio_manager.h"

int main(void)
{
    const int screenWidth = 1280;
    const int screenHeight = 720;

    InitWindow(
        screenWidth,
        screenHeight,
        "Re_7"
        AudioManagerInit();
    );

    SetTargetFPS(60);

    // ESC nao fecha mais o jogo (ele volta ao menu)
    SetExitKey(KEY_NULL);

    Assets assets;

    LoadAssets(&assets);

    GameState game;

    InitGame(&game);


    while (!WindowShouldClose() && !game.shouldExit)
    {
        UpdateGame(&game);

        BeginDrawing();

        ClearBackground(BLACK);

        DrawGame(&game, &assets);

        EndDrawing();
    }


    UnloadAssets(&assets);

    AudioManagerClose();
    CloseWindow();

    return 0;
}

/* Iniciar no terminal:
gcc src/main.c src/game.c src/ui.c src/scenes.c src/assets.c src/story.c src/audio_manager.c -o re7.exe -IC:/raylib/raylib/src -Isrc -LC:/raylib/raylib/src -lraylib -lopengl32 -lgdi32 -lwinmm

.\re7.exe

*/