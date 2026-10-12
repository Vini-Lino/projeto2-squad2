#ifndef ASSETS_H
#define ASSETS_H

#include "raylib.h"


typedef struct
{

    // backgrounds

    Texture2D bgPresent;

    Texture2D bg2007;

    Texture2D bgRua;


    // botões do menu: [botão][estado]
    // botão: 0 novo jogo, 1 continuar, 2 créditos, 3 sair
    // estado: 0 normal, 1 hover, 2 pressed

    Texture2D menuButtons[4][3];

    Texture2D logo;

    // avatar da IA (estrela do logo)

    Texture2D iaStar;


    // personagens

    Texture2D liaJovemLado;
    Texture2D liaFrente;

} Assets;


void LoadAssets(Assets *assets);

void UnloadAssets(Assets *assets);


#endif