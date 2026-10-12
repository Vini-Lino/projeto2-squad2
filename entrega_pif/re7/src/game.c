#include "game.h"

#include "ui.h"
#include "scenes.h"
#include "minigame.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


// =====================================================
// FUNDO COSMICO DO MENU
// =====================================================

static void BrilhoSuave(float centroX, float centroY, float raio, Color cor, float opacidade)
{
    DrawCircleGradient((Vector2){centroX, centroY}, raio, Fade(cor, opacidade), Fade(cor, 0.0f));
}

static void DesenharFundo(void)
{
    int larguraTela = GetScreenWidth();
    int alturaTela = GetScreenHeight();
    float tempo = (float)GetTime();

    // base: violeta-noite para azul-marinho
    DrawRectangleGradientV(0, 0, larguraTela, alturaTela, (Color){24, 14, 52, 255}, (Color){12, 34, 66, 255});

    // manchas de luz nas cores do logo, flutuando devagar
    BrilhoSuave(larguraTela * 0.20f + sinf(tempo * 0.30f) * 30, alturaTela * 0.30f, 340, (Color){255, 105, 200, 255}, 0.35f);
    BrilhoSuave(larguraTela * 0.80f + cosf(tempo * 0.25f) * 30, alturaTela * 0.70f, 380, (Color){ 80, 200, 255, 255}, 0.30f);
    BrilhoSuave(larguraTela * 0.50f,                        alturaTela * 0.40f, 300, (Color){190, 150, 255, 255}, 0.25f);

    // estrelinhas piscando
    for (int i = 0; i < 45; i++)
    {
        unsigned int semente = (unsigned int)(i + 1) * 2654435761u;
        float posX = (float)(semente % (unsigned)larguraTela);
        float posY = (float)((semente >> 8) % (unsigned)alturaTela);
        float tamanho = 3.0f + (float)((semente >> 16) % 5);
        float brilho = 0.35f + 0.65f * (0.5f + 0.5f * sinf(tempo * 2.0f + i));
        Color corEstrela = Fade(WHITE, brilho);

        DrawLineEx((Vector2){posX - tamanho, posY}, (Vector2){posX + tamanho, posY}, 1.5f, corEstrela);
        DrawLineEx((Vector2){posX, posY - tamanho}, (Vector2){posX, posY + tamanho}, 1.5f, corEstrela);
        DrawCircleV((Vector2){posX, posY}, 1.5f, corEstrela);
    }
}


// =====================================================
// SAVE / LOAD (arquivo de texto: "cena fala")
// =====================================================

#define SAVE_FILE "save.dat"

static void SaveGame(GameState *game)
{
    SaveFileText(
        SAVE_FILE,
        (char *)TextFormat("%d %d", (int)game->currentScene, game->dialogueIndex)
    );
}

// Retorna true se conseguiu carregar um save valido
static bool LoadGame(GameState *game)
{
    if (!FileExists(SAVE_FILE)) return false;

    char *text = LoadFileText(SAVE_FILE);

    if (text == NULL) return false;

    int scene = 0;
    int index = 0;

    int lidos = sscanf(text, "%d %d", &scene, &index);

    UnloadFileText(text);

    if (lidos != 2) return false;

    if (scene < 0 || scene >= SCENE_COUNT) return false;

    Scene s = GetScene((SceneID)scene);

    if (index < 0 || index >= s.dialogueCount) index = 0;

    game->currentScene = (SceneID)scene;
    game->dialogueIndex = index;

    return true;
}


// =====================================================
// EFEITO DE VIAGEM NO TEMPO (flash) E PERSONAGEM
// =====================================================

#define FLASH_DURACAO 1.4f

// Tempo restante do flash (0 = sem flash)
static float flashTimer = 0.0f;

// Tempo restante da mensagem "opcao invalida" na tela final
static float avisoInvalido = 0.0f;

// Desenha uma personagem mantendo a proporcao da imagem.
// altura: altura na tela | yTopo: posicao vertical | xDireita: borda direita
static void DrawPersonagem(Texture2D tex, float altura, float yTopo, float xDireita)
{
    float largura = altura * (float)tex.width / (float)tex.height;

    Rectangle destino = { xDireita - largura, yTopo, largura, altura };

    DrawTexturePro(
        tex,
        (Rectangle){ 0, 0, (float)tex.width, (float)tex.height },
        destino,
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}

// Flash branco-rosado com "glitch" que se apaga aos poucos
static void DesenharFlash(void)
{
    if (flashTimer <= 0.0f) return;

    float intensidade = flashTimer / FLASH_DURACAO;      // 1 -> 0

    DrawRectangle(
        0, 0, GetScreenWidth(), GetScreenHeight(),
        Fade((Color){255, 235, 250, 255}, intensidade)
    );

    // listras de glitch no comeco do efeito
    if (intensidade > 0.55f)
    {
        for (int i = 0; i < 10; i++)
        {
            int posY = GetRandomValue(0, GetScreenHeight() - 20);
            int alturaListra = GetRandomValue(4, 18);
            int posX = GetRandomValue(0, 300);

            Color corListra = (i % 2 == 0) ? (Color){255, 105, 200, 255} : (Color){80, 200, 255, 255};

            DrawRectangle(posX, posY, GetScreenWidth() - posX, alturaListra, Fade(corListra, 0.5f * intensidade));
        }
    }
}

// Avatar da IA: a estrela do logo, flutuando ao lado da caixa de dialogo
static void DesenharAvatarIA(Assets *assets)
{
    float bob = sinf((float)GetTime() * 3.0f) * 5.0f;

    float cx = 200.0f;
    float cy = 470.0f + bob;

    DrawCircleGradient(
        (Vector2){cx, cy}, 95.0f,
        Fade((Color){255, 105, 200, 255}, 0.45f),
        Fade((Color){255, 105, 200, 255}, 0.0f)
    );

    DrawTexturePro(
        assets->iaStar,
        (Rectangle){ 0, 0, (float)assets->iaStar.width, (float)assets->iaStar.height },
        (Rectangle){ cx - 60.0f, cy - 60.0f, 120.0f, 120.0f },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );
}


// =====================================================
// POSIÇÕES DOS BOTÕES DO MENU
// =====================================================

#define MENU_BUTTONS 4

static Rectangle menuRects[MENU_BUTTONS] =
{
    {490, 439, 299, 59},   // 0 - Novo Jogo
    {515, 505, 250, 50},   // 1 - Continuar
    {515, 565, 250, 50},   // 2 - Creditos
    {515, 625, 250, 50}    // 3 - Sair
};


// =====================================================
// INICIALIZAR JOGO
// =====================================================

void InitGame(GameState *game)
{
    game->currentScreen = SCREEN_MENU;

    game->currentScene = SCENE_PRESENT;

    game->dialogueIndex = 0;

    game->shouldExit = false;
}


// =====================================================
// TROCAR DE CENA
// =====================================================

void ChangeScene(
    GameState *game,
    SceneID nextScene
)
{
    game->currentScene = nextScene;

    game->dialogueIndex = 0;
}


// =====================================================
// COMECAR NOVO JOGO
// (usada no menu e na tela final, para nao repetir o bloco)
// =====================================================

static void ComecarNovoJogo(GameState *game)
{
    game->currentScreen = SCREEN_GAME;

    ChangeScene(game, SCENE_PRESENT);

    flashTimer = 0.0f;

    SaveGame(game);                                 // zera o save
}


// =====================================================
// UPDATE
// =====================================================

void UpdateGame(GameState *game)
{
    // o flash de viagem no tempo vai se apagando
    if (flashTimer > 0.0f) flashTimer -= GetFrameTime();

    // -------------------------------------------------
    // MENU
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MENU)
    {
        if (IsButtonClicked(menuRects[0]))          // Novo Jogo
        {
            ComecarNovoJogo(game);
        }

        if (IsButtonClicked(menuRects[1]))          // Continuar
        {
            // carrega o save; se nao houver, comeca do inicio
            if (!LoadGame(game))
            {
                ChangeScene(game, SCENE_PRESENT);
            }

            flashTimer = 0.0f;

            game->currentScreen = SCREEN_GAME;
        }

        if (IsButtonClicked(menuRects[2]))          // Creditos
        {
            game->currentScreen = SCREEN_CREDITS;
        }

        if (IsButtonClicked(menuRects[3]))          // Sair
        {
            game->shouldExit = true;
        }

        return;
    }


    // -------------------------------------------------
    // CREDITOS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CREDITS)
    {
        if (
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
            ||
            IsKeyPressed(KEY_ENTER)
            ||
            IsKeyPressed(KEY_BACKSPACE)
            ||
            IsKeyPressed(KEY_ESCAPE)
        )
        {
            game->currentScreen = SCREEN_MENU;
        }

        return;
    }


    // -------------------------------------------------
    // TELA FINAL: jogar de novo, menu ou sair
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENDING)
    {
        if (avisoInvalido > 0.0f) avisoInvalido -= GetFrameTime();

        int tecla;                  // tecla apertada neste quadro

        // le TODAS as teclas apertadas neste quadro
        // e valida cada uma (so ENTER, M e ESC sao aceitas)
        while ((tecla = GetKeyPressed()) != 0)
        {
            if (tecla == KEY_ENTER)
            {
                ComecarNovoJogo(game);

                avisoInvalido = 0.0f;
            }
            else if (tecla == KEY_M)
            {
                game->currentScreen = SCREEN_MENU;

                avisoInvalido = 0.0f;
            }
            else if (tecla == KEY_ESCAPE)
            {
                game->shouldExit = true;
            }
            else
            {
                avisoInvalido = 1.5f;               // opcao invalida
            }
        }

        return;
    }


    // -------------------------------------------------
    // MINIGAME
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MINIGAME)
    {
        MinigameResult resultado = UpdateMinigame();

        if (resultado == MINIGAME_MENU)             // ESC
        {
            game->currentScreen = SCREEN_MENU;
        }
        else if (resultado == MINIGAME_VENCEU)      // completou o desafio
        {
            game->currentScreen = SCREEN_GAME;

            ChangeScene(game, SCENE_2007_WIN);

            flashTimer = FLASH_DURACAO;

            SaveGame(game);
        }

        return;
    }


    // -------------------------------------------------
    // JOGO (HISTORIA)
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_GAME)
    {
        // ESC: salva e volta ao menu
        if (IsKeyPressed(KEY_ESCAPE))
        {
            SaveGame(game);

            game->currentScreen = SCREEN_MENU;

            return;
        }

        Scene scene = GetScene(game->currentScene);

        if (IsKeyPressed(KEY_SPACE))
        {
            game->dialogueIndex++;

            if (game->dialogueIndex >= scene.dialogueCount)
            {
                if (game->currentScene == SCENE_2007)
                {
                    // fim da cena de 2007: comeca o minigame.
                    // O save fica na ultima fala, para o Continuar voltar aqui.
                    game->dialogueIndex = scene.dialogueCount - 1;

                    SaveGame(game);

                    InitMinigame();

                    game->currentScreen = SCREEN_MINIGAME;

                    return;
                }

                if (game->currentScene == SCENE_EPILOGUE)
                {
                    // fim da historia: volta ao inicio (para jogar de novo)
                    // e mostra a tela final
                    ChangeScene(game, SCENE_PRESENT);

                    SaveGame(game);

                    game->currentScreen = SCREEN_ENDING;

                    return;
                }

                ChangeScene(game, scene.nextScene);

                // viagem no tempo: flash ao chegar em 2007 ou ao voltar ao presente
                if (game->currentScene == SCENE_2007 || game->currentScene == SCENE_EPILOGUE)
                    flashTimer = FLASH_DURACAO;
            }

            SaveGame(game);                         // salva a cada fala
        }
    }
}


// =====================================================
// DESENHAR JOGO
// =====================================================

void DrawGame(
    GameState *game,
    Assets *assets
)
{

    // -------------------------------------------------
    // MENU
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MENU)
    {
        DesenharFundo();

        // X, Y, largura, altura
        // (aumentar o Y desce o logo, diminuir sobe)
        Rectangle logoRect = {280, 100, 719, 319};

        DrawTexturePro(
            assets->logo,
            (Rectangle){
                0,
                0,
                assets->logo.width,
                assets->logo.height
            },
            logoRect,
            (Vector2){0, 0},
            0.0f,
            WHITE
        );

        for (int i = 0; i < MENU_BUTTONS; i++)
        {
            DrawImageButton(
                assets->menuButtons[i][0],
                assets->menuButtons[i][1],
                assets->menuButtons[i][2],
                menuRects[i]
            );
        }

        return;
    }


    // -------------------------------------------------
    // CREDITOS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_CREDITS)
    {
        DesenharFundo();

        DrawCenteredText("CREDITOS", 50, 48, WHITE);
        DrawCenteredText("Re:7 - Projeto Integrador", 115, 26, SKYBLUE);

        // nomes da equipe (um por linha, centralizados)
        static const char *nomesEquipe[] =
        {
            "Aline Santos",
            "Ana Rita Correia",
            "Gabriel Farias",
            "Juan Nunes",
            "Juliana Freire",
            "Leandro Luís",
            "Luiz Mario",
            "Vinícius Lino",
            "Vitória Santos"
        };

        int totalNomes = (int)(sizeof(nomesEquipe) / sizeof(nomesEquipe[0]));

        for (int i = 0; i < totalNomes; i++)
        {
            DrawCenteredText(nomesEquipe[i], 190 + i * 41, 30, WHITE);
        }

        DrawCenteredText("Clique, ENTER ou ESC para voltar", 640, 20, Fade(WHITE, 0.7f));

        return;
    }


    // -------------------------------------------------
    // TELA FINAL
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_ENDING)
    {
        DesenharFundo();

        DrawCenteredText("FIM", 150, 90, WHITE);
        DrawCenteredText("Lia aprendeu a confiar na propria voz.", 280, 30, SKYBLUE);
        DrawCenteredText("Obrigada por jogar Re:7!", 330, 28, WHITE);

        DrawCenteredText("ENTER - jogar novamente", 440, 28, WHITE);
        DrawCenteredText("M - voltar ao menu", 485, 28, WHITE);
        DrawCenteredText("ESC - sair do jogo", 530, 28, WHITE);

        if (avisoInvalido > 0.0f)
        {
            DrawCenteredText(
                "Opcao invalida! Use ENTER, M ou ESC.",
                600, 24,
                Fade((Color){255, 105, 200, 255}, fminf(1.0f, avisoInvalido))
            );
        }

        return;
    }


    // -------------------------------------------------
    // MINIGAME
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_MINIGAME)
    {
        DrawMinigame();

        return;
    }


    // -------------------------------------------------
    // CENAS
    // -------------------------------------------------

    if (game->currentScreen == SCREEN_GAME)
    {
        Texture2D background;

        switch (game->currentScene)
        {
            case SCENE_PRESENT:
                background = assets->bgPresent;
                break;

            case SCENE_2007:
            case SCENE_2007_WIN:
                background = assets->bg2007;
                break;

            default:
                background = assets->bgPresent;
                break;
        }

        // Background ocupando a tela toda

        DrawTexturePro(
            background,
            (Rectangle){
                0,
                0,
                background.width,
                background.height
            },
            (Rectangle){
                0,
                0,
                GetScreenWidth(),
                GetScreenHeight()
            },
            (Vector2){0, 0},
            0,
            WHITE
        );

        // Personagem: Lia adulta no presente, Lia jovem em 2007
        // (altura, posicao Y do topo, borda direita)

        if (game->currentScene == SCENE_2007 || game->currentScene == SCENE_2007_WIN)
        {
            DrawPersonagem(assets->liaJovemLado, 900.0f, -101.0f, 1250.0f);
        }
        else
        {
            DrawPersonagem(assets->liaFrente, 820.0f, -7.0f, 1250.0f);
        }

        // ---------------------------------------------
        // DIÁLOGO
        // ---------------------------------------------

        Scene scene = GetScene(game->currentScene);

        Dialogue dialogue = scene.dialogues[game->dialogueIndex];

        DrawDialogueBox(
            dialogue.speaker,
            dialogue.text
        );

        // ---------------------------------------------
        // AVISO: ESC VOLTA AO MENU
        // ---------------------------------------------

        DrawRectangleRounded(
            (Rectangle){20, 20, 170, 36},
            0.4f,
            8,
            Fade(BLACK, 0.55f)
        );

        DrawText("ESC  MENU", 38, 29, 20, WHITE);

        // ---------------------------------------------
        // AVATAR DA IA (so quando a IA esta falando)
        // ---------------------------------------------

        if (strcmp(dialogue.speaker, "IA") == 0)
        {
            DesenharAvatarIA(assets);
        }

        // ---------------------------------------------
        // FLASH DA VIAGEM NO TEMPO
        // ---------------------------------------------

        DesenharFlash();
    }


    // Para desenvolvimento:

    // DrawDebugGrid();

    // DrawMousePosition();
}