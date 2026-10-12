#include "ui.h"


// =====================================================
// TEXTO CENTRALIZADO
// =====================================================

void DrawCenteredText(
    const char *text,
    int y,
    int fontSize,
    Color color
)
{
    int width =
        MeasureText(
            text,
            fontSize
        );


    int x =
        (
            GetScreenWidth()
            -
            width
        )
        /
        2;


    DrawText(
        text,
        x,
        y,
        fontSize,
        color
    );
}


// =====================================================
// BOTÃO
// =====================================================

void DrawImageButton(
    Texture2D normal,
    Texture2D hover,
    Texture2D pressed,
    Rectangle rect
)
{
    Vector2 mouse =
        GetMousePosition();


    bool mouseOver =
        CheckCollisionPointRec(
            mouse,
            rect
        );


    Texture2D texture =
        normal;


    if (mouseOver)
    {
        texture =
            hover;
    }


    if (
        mouseOver
        &&
        IsMouseButtonDown(
            MOUSE_BUTTON_LEFT
        )
    )
    {
        texture =
            pressed;
    }


    DrawTexturePro(

        texture,

        (Rectangle)
        {
            0,
            0,
            texture.width,
            texture.height
        },

        rect,

        (Vector2)
        {
            0,
            0
        },

        0,

        WHITE
    );
}


// =====================================================
// DETECTAR CLIQUE
// =====================================================

bool IsButtonClicked(
    Rectangle rect
)
{
    Vector2 mouse =
        GetMousePosition();


    return
        CheckCollisionPointRec(
            mouse,
            rect
        )

        &&

        IsMouseButtonPressed(
            MOUSE_BUTTON_LEFT
        );
}


// =====================================================
// CAIXA DE DIÁLOGO
// =====================================================

void DrawDialogueBox(
    const char *speaker,
    const char *text
)
{

    Rectangle box =
    {
        120,
        520,
        1040,
        160
    };


    DrawRectangleRounded(
        box,
        0.08f,
        10,
        Fade(BLACK, 0.80f)
    );


    DrawText(
        speaker,
        160,
        545,
        26,
        PINK
    );


    DrawText(
        text,
        160,
        590,
        28,
        WHITE
    );


    DrawText(
        ">>",
        1080,
        635,
        28,
        SKYBLUE
    );
}


// =====================================================
// DEBUG GRID
// =====================================================

void DrawDebugGrid(void)
{
    for (
        int x = 0;
        x < GetScreenWidth();
        x += 100
    )
    {
        DrawLine(
            x,
            0,
            x,
            GetScreenHeight(),
            Fade(WHITE, 0.25f)
        );
    }


    for (
        int y = 0;
        y < GetScreenHeight();
        y += 100
    )
    {
        DrawLine(
            0,
            y,
            GetScreenWidth(),
            y,
            Fade(WHITE, 0.25f)
        );
    }
}


// =====================================================
// POSIÇÃO DO MOUSE
// =====================================================

void DrawMousePosition(void)
{
    Vector2 mouse =
        GetMousePosition();


    DrawText(

        TextFormat(
            "X: %.0f Y: %.0f",
            mouse.x,
            mouse.y
        ),

        20,
        20,
        20,
        YELLOW
    );
}