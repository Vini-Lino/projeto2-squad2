#ifndef UI_H
#define UI_H

#include "raylib.h"


void DrawCenteredText(
    const char *text,
    int y,
    int fontSize,
    Color color
);


void DrawImageButton(
    Texture2D normal,
    Texture2D hover,
    Texture2D pressed,
    Rectangle rect
);


bool IsButtonClicked(
    Rectangle rect
);


void DrawDialogueBox(
    const char *speaker,
    const char *text
);


void DrawDebugGrid(void);

void DrawMousePosition(void);


#endif