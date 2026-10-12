#ifndef MINIGAME_H
#define MINIGAME_H

#include <stdbool.h>


// O que aconteceu no quadro atual do minigame

typedef enum
{
    MINIGAME_CONTINUA,      // segue jogando
    MINIGAME_MENU,          // a jogadora apertou ESC: volta ao menu
    MINIGAME_VENCEU         // completou o desafio: segue a historia
} MinigameResult;


// Comeca (ou recomeca) o minigame do zero.

void InitMinigame(void);


// Atualiza o minigame (chamar uma vez por quadro).

MinigameResult UpdateMinigame(void);


// Desenha o minigame (chamar entre BeginDrawing e EndDrawing).

void DrawMinigame(void);


#endif