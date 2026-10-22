#include "room.h"

#include "ui.h"
#include "scenes.h"
#include "audio_manager.h"
#include <math.h>


// =====================================================
// EXPLORACAO DO QUARTO (2007) - US08
//
// A jogadora clica em objetos do quarto e le um pequeno
// trecho na caixa de dialogo da visual novel. Ao terminar,
// volta a exploracao. Os objetos podem ser relidos.
// Enquanto um trecho esta aberto, os outros objetos nao
// recebem cliques.
//
// Regra de exploracao: LIVRE e OPCIONAL, sem ordem. O botao
// "Continuar" fica disponivel o tempo todo e leva a proxima
// parte da historia.
// =====================================================


#define TOTAL_PONTOS        3       // quantidade de objetos clicaveis
#define FALAS_POR_PONTO     2       // maximo de falas de cada trecho
#define RAIO_PONTO          38.0f   // tamanho da area clicavel (em pixels)
#define NENHUM_PONTO        -1      // valor de "nenhum trecho aberto"


// Um objeto clicavel do quarto
typedef struct
{
    const char *nome;                   // nome mostrado ao passar o mouse
    int totalFalas;                     // quantas falas o trecho tem
    Dialogue falas[FALAS_POR_PONTO];    // trecho mostrado na caixa de dialogo
    float posicaoX;                     // posicao horizontal (0.0 = esquerda, 1.0 = direita)
    float posicaoY;                     // posicao vertical (0.0 = topo, 1.0 = base)
} PontoQuarto;


// Objetos do quarto (posicoes ajustadas para a imagem quarto_2007.png).
// Se trocar a imagem ou o ponto nao ficar em cima do objeto, ajuste
// posicaoX e posicaoY aqui (veja a tecla F1 no jogo).
static const PontoQuarto pontosDoQuarto[TOTAL_PONTOS] =
{
    // Poster: revela um interesse da protagonista e ambienta 2007
    {
        "Poster",
        2,
        {
            { "Lia", "O poster do Chat Noir, o gato preto. Comprei na feira." },
            { "Lia", "Em 2007 eu queria um quarto com cara de artista." }
        },
        0.055f, 0.170f
    },

    // Diario na cama: revela um sentimento da protagonista
    {
        "Diario",
        2,
        {
            { "Lia", "Meu diario, esquecido em cima da cama." },
            { "Lia", "\"Amanha eu apresento. E se eu travar na frente de todos?\"" }
        },
        0.200f, 0.570f
    },

    // PC: revela a vida digital da protagonista
    {
        "Computador",
        2,
        {
            { "Lia", "O MSN esta aberto e o Orkut esta na outra aba." },
            { "Lia", "Passo horas aqui, mas hoje so penso no trabalho de amanha." }
        },
        0.885f, 0.360f
    }
};


// =====================================================
// ESTADO DA EXPLORACAO (fica guardado entre os quadros)
// =====================================================

// true quando o objeto ja foi aberto pelo menos uma vez
static bool pontoVisitado[TOTAL_PONTOS];

// indice do objeto aberto (NENHUM_PONTO se nenhum trecho esta aberto)
static int pontoAberto = NENHUM_PONTO;

// qual fala do trecho aberto esta sendo mostrada
static int falaAtual = 0;

// true enquanto a ajuda de desenvolvimento (tecla F1) estiver ligada
static bool mostrarAjuda = false;


// =====================================================
// POSICOES NA TELA
// =====================================================

// Centro de um objeto, em pixels (a posicao 0 a 1 vira pixel)
static Vector2 CentroDoPonto(int indice)
{
    return (Vector2){
        pontosDoQuarto[indice].posicaoX * (float)GetScreenWidth(),
        pontosDoQuarto[indice].posicaoY * (float)GetScreenHeight()
    };
}

// Botao "Continuar" (sempre disponivel durante a exploracao)
static Rectangle AreaBotaoContinuar(void)
{
    float largura = 300.0f;
    float altura = 60.0f;

    return (Rectangle){
        ((float)GetScreenWidth() - largura) / 2.0f,
        (float)GetScreenHeight() - altura - 36.0f,
        largura,
        altura
    };
}

// Indice do objeto sob o mouse, ou NENHUM_PONTO
static int PontoSobOMouse(void)
{
    Vector2 mouse = GetMousePosition();

    for (int indice = 0; indice < TOTAL_PONTOS; indice++)
    {
        if (CheckCollisionPointCircle(mouse, CentroDoPonto(indice), RAIO_PONTO))
        {
            return indice;
        }
    }

    return NENHUM_PONTO;
}


// =====================================================
// INICIAR
// =====================================================

void InitRoom(void)
{
    for (int indice = 0; indice < TOTAL_PONTOS; indice++)
    {
        pontoVisitado[indice] = false;
    }

    pontoAberto = NENHUM_PONTO;
    falaAtual = 0;
    mostrarAjuda = false;
}


// =====================================================
// UPDATE
// =====================================================

RoomResult UpdateRoom(void)
{
    // F1 liga e desliga a ajuda de desenvolvimento
    if (IsKeyPressed(KEY_F1)) mostrarAjuda = !mostrarAjuda;

    // ---------- trecho aberto: so ele recebe os comandos ----------

    if (pontoAberto != NENHUM_PONTO)
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        // ESPACO, ENTER ou clique passam para a proxima fala
        if (
            IsKeyPressed(KEY_SPACE)
            ||
            IsKeyPressed(KEY_ENTER)
            ||
            IsMouseButtonPressed(MOUSE_BUTTON_LEFT)
        )
        {
            AudioManager_Play(SOM_DIALOGO);

            falaAtual++;

            // acabou o trecho: volta a exploracao
            if (falaAtual >= pontosDoQuarto[pontoAberto].totalFalas)
            {
                pontoAberto = NENHUM_PONTO;
            }
        }

        // ESC fecha o trecho e volta a exploracao
        if (IsKeyPressed(KEY_ESCAPE))
        {
            pontoAberto = NENHUM_PONTO;
        }

        return ROOM_CONTINUA;
    }

    // ---------- exploracao livre ----------

    if (IsKeyPressed(KEY_ESCAPE))
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);

        return ROOM_MENU;
    }

    int pontoSobMouse = PontoSobOMouse();

    bool sobreBotaoContinuar = CheckCollisionPointRec(GetMousePosition(), AreaBotaoContinuar());

    // a mao aparece sobre qualquer coisa clicavel
    if (pontoSobMouse != NENHUM_PONTO || sobreBotaoContinuar)
    {
        SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
    }
    else
    {
        SetMouseCursor(MOUSE_CURSOR_DEFAULT);
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
    {
        if (pontoSobMouse != NENHUM_PONTO)
        {
            // abre o trecho do objeto (pode ser relido quantas vezes quiser)
            AudioManager_Play(SOM_PASSADO);

            pontoAberto = pontoSobMouse;
            falaAtual = 0;
            pontoVisitado[pontoSobMouse] = true;
        }
        else if (sobreBotaoContinuar)
        {
            AudioManager_Play(SOM_MOUSE);

            SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            return ROOM_CONCLUIDO;
        }
    }

    return ROOM_CONTINUA;
}


// =====================================================
// DESENHO
// =====================================================

// Desenha um objeto clicavel (anel que pulsa; fica azul e com "OK" depois de visto)
static void DesenharPonto(int indice, bool sobOMouse)
{
    Vector2 centro = CentroDoPonto(indice);

    float pulso = 0.5f + 0.5f * sinf((float)GetTime() * 3.0f + (float)indice);
    float raio = RAIO_PONTO * (sobOMouse ? 1.2f : (0.9f + 0.1f * pulso));

    Color corBase = pontoVisitado[indice] ? (Color){ 80, 200, 255, 255 } : (Color){ 255, 105, 200, 255 };

    // brilho suave ao redor
    DrawCircleGradient(centro, raio * 1.8f, Fade(corBase, sobOMouse ? 0.55f : 0.35f), Fade(corBase, 0.0f));

    // miolo e anel
    DrawCircleV(centro, raio * 0.8f, Fade(BLACK, 0.45f));
    DrawRing(centro, raio * 0.8f - 4.0f, raio * 0.8f, 0.0f, 360.0f, 40, corBase);

    // simbolo no meio
    const char *simbolo = pontoVisitado[indice] ? "OK" : "?";
    int larguraSimbolo = MeasureText(simbolo, 28);

    DrawText(simbolo, (int)centro.x - larguraSimbolo / 2, (int)centro.y - 14, 28, WHITE);
}

void DrawRoom(Assets *assets)
{
    // fundo: o quarto de 2007, ocupando a tela toda
    DrawTexturePro(
        assets->bg2007,
        (Rectangle){ 0, 0, (float)assets->bg2007.width, (float)assets->bg2007.height },
        (Rectangle){ 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() },
        (Vector2){ 0, 0 },
        0.0f,
        WHITE
    );

    bool trechoAberto = (pontoAberto != NENHUM_PONTO);

    int pontoSobMouse = trechoAberto ? NENHUM_PONTO : PontoSobOMouse();

    // objetos clicaveis
    for (int indice = 0; indice < TOTAL_PONTOS; indice++)
    {
        DesenharPonto(indice, indice == pontoSobMouse);
    }

    // nome do objeto quando o mouse esta em cima
    if (pontoSobMouse != NENHUM_PONTO)
    {
        Vector2 centro = CentroDoPonto(pontoSobMouse);
        const char *nome = pontosDoQuarto[pontoSobMouse].nome;
        int larguraNome = MeasureText(nome, 22);

        DrawRectangleRounded(
            (Rectangle){ centro.x - larguraNome / 2.0f - 12.0f, centro.y - RAIO_PONTO - 48.0f, larguraNome + 24.0f, 34.0f },
            0.4f, 8, Fade(BLACK, 0.7f)
        );

        DrawText(nome, (int)centro.x - larguraNome / 2, (int)(centro.y - RAIO_PONTO - 41.0f), 22, WHITE);
    }

    // faixa de cima: instrucao
    DrawRectangle(0, 0, GetScreenWidth(), 64, Fade(BLACK, 0.6f));

    DrawText("Explore o quarto (opcional)", 24, 12, 26, WHITE);

    DrawText("Clique nos objetos para ver mais. Quando quiser, clique em Continuar.", 24, 40, 18, SKYBLUE);

    DrawText("ESC  MENU", GetScreenWidth() - 150, 22, 20, WHITE);

    if (trechoAberto)
    {
        // trecho do objeto na caixa de dialogo da visual novel
        Dialogue fala = pontosDoQuarto[pontoAberto].falas[falaAtual];

        DrawDialogueBox(fala.speaker, fala.text);
    }
    else
    {
        // botao Continuar (sempre disponivel; some enquanto um trecho esta aberto)
        Rectangle botao = AreaBotaoContinuar();
        bool sobOBotao = CheckCollisionPointRec(GetMousePosition(), botao);

        DrawRectangleRounded(botao, 0.5f, 12, sobOBotao ? (Color){ 255, 105, 200, 255 } : (Color){ 190, 60, 150, 255 });
        DrawRectangleRoundedLines(botao, 0.5f, 12, WHITE);

        const char *textoBotao = "Continuar";
        int larguraTextoBotao = MeasureText(textoBotao, 28);

        DrawText(textoBotao, (int)(botao.x + (botao.width - larguraTextoBotao) / 2.0f), (int)(botao.y + 16.0f), 28, WHITE);
    }

    // ajuda de desenvolvimento (F1): mostra onde o mouse esta em escala 0 a 1
    if (mostrarAjuda)
    {
        Vector2 mouse = GetMousePosition();

        DrawText(
            TextFormat("F1 ajuda | mouse: x=%.3f  y=%.3f", mouse.x / (float)GetScreenWidth(), mouse.y / (float)GetScreenHeight()),
            24, 76, 20, YELLOW
        );
    }
}