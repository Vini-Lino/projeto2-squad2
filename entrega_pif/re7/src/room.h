#ifndef ROOM_H
#define ROOM_H

#include "raylib.h"

#include "assets.h"


// O que aconteceu no quadro atual da exploracao do quarto

typedef enum
{
    ROOM_CONTINUA,      // segue explorando
    ROOM_MENU,          // a jogadora apertou ESC: volta ao menu
    ROOM_CONCLUIDO      // explorou tudo e clicou em continuar
} RoomResult;


// Comeca a exploracao do quarto do zero (nenhum ponto visitado).

void InitRoom(void);


// Atualiza a exploracao (chamar uma vez por quadro).

RoomResult UpdateRoom(void);


// Desenha o quarto, os pontos clicaveis e o popup (chamar entre BeginDrawing e EndDrawing).

void DrawRoom(Assets *assets);


#endif