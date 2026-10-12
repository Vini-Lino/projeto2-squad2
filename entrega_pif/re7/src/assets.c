#include "assets.h"


void LoadAssets(Assets *assets)
{

    // BACKGROUNDS

    assets->bgPresent = LoadTexture("assets/backgrounds/quarto_presente.png");

    assets->bg2007 = LoadTexture("assets/backgrounds/quarto_2007.png");

    assets->bgRua = LoadTexture("assets/backgrounds/rua.png");


    // BOTOES DO MENU (normal, hover, pressed)

    static const char *menuPaths[4][3] =
    {
        {"assets/ui/novoJogo.png",  "assets/ui/novoJogoHover.png",  "assets/ui/novoJogoPressed.png"},
        {"assets/ui/continuar.png", "assets/ui/continuarHover.png", "assets/ui/continuarPressed.png"},
        {"assets/ui/creditos.png",  "assets/ui/creditosHover.png",  "assets/ui/creditosPressed.png"},
        {"assets/ui/sair.png",      "assets/ui/sairHover.png",      "assets/ui/sairPressed.png"}
    };

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            assets->menuButtons[i][j] = LoadTexture(menuPaths[i][j]);


    // LOGO

    assets->logo = LoadTexture("assets/ui/logo.png");

    // AVATAR DA IA

    assets->iaStar = LoadTexture("assets/ui/iaStar.png");


    // PERSONAGEM

    assets->liaFrente = LoadTexture("assets/characters/liaFrente.png");

    assets->liaJovemLado = LoadTexture("assets/characters/liaJovemLado.png");
}


void UnloadAssets(Assets *assets)
{
    UnloadTexture(assets->bgPresent);
    UnloadTexture(assets->bg2007);
    UnloadTexture(assets->bgRua);

    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 3; j++)
            UnloadTexture(assets->menuButtons[i][j]);

    UnloadTexture(assets->liaFrente);
    UnloadTexture(assets->liaJovemLado);
    UnloadTexture(assets->logo);
    UnloadTexture(assets->iaStar);
}