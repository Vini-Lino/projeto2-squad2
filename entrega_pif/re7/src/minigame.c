/*
 * Minigame de obstaculos + puzzle (adaptado do Mini-game-3-C).
 *
 * Fluxo: corrida de obstaculos -> (3000 pontos) -> puzzle de 3 fases -> vitoria.
 *
 * Controles:
 *   Corrida: ESPACO, W ou seta para cima para pular
 *   Puzzle:  WASD/setas movem, ESPACO atira, R reinicia a fase
 *   Fim:     ENTER joga de novo, ESC volta ao menu
 *   Vitoria: ENTER continua a historia
 *   ESC em qualquer tela do minigame volta ao menu do jogo.
 *
 * O minigame foi feito para uma janela de 800x450. Ele continua desenhando
 * nessas medidas, e a camera amplia tudo para a janela do jogo (1280x720).
 */

#include "minigame.h"

#include "raylib.h"
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define LARGURA          800
#define ALTURA           450
#define CHAO_Y           360

#define JOG_X            100.0f
#define JOG_LARGURA      40.0f
#define JOG_ALTURA       50.0f

#define GRAVIDADE        2200.0f
#define FORCA_PULO       780.0f
#define VEL_INICIAL      400.0f
#define VEL_MAXIMA       920.0f
#define ACELERACAO       20.0f

#define MAX_OBSTACULOS   8
#define BONUS_OBSTACULO  25.0f
#define PONTOS_POR_SEG   10.0f

#define ARQUIVO_RECORDE  "recorde.dat"

#define PONTOS_TRANSICAO    3000.0f

#define PUZZLE_AREA_X       40
#define PUZZLE_AREA_Y       90
#define PUZZLE_AREA_LARGURA 720
#define PUZZLE_AREA_ALTURA  300
#define PUZZLE_VEL          220.0f
#define BALA_VEL            520.0f
#define MAX_BLOCOS          6
#define MAX_BALAS           5
#define BALAS_POR_FASE      3
#define TOTAL_FASES         3

/* Qual tela do minigame esta ativa */
typedef enum { TELA_JOGO, TELA_FIM, TELA_PUZZLE, TELA_VITORIA } Tela;
/* O que aconteceu na corrida neste quadro */
typedef enum { RESULTADO_NADA, RESULTADO_COLISAO, RESULTADO_TRANSICAO } ResultadoCorrida;
/* O que aconteceu no puzzle neste quadro */
typedef enum { PUZZLE_NADA, PUZZLE_SAIU } ResultadoPuzzle;

/* Jogador da corrida: corpo (posicao e tamanho), velocidade vertical e se esta no chao */
typedef struct {
    Rectangle corpo;
    float velY;
    bool noChao;
} Jogador;

/* Obstaculo da corrida; "contado" evita dar o bonus de pontos duas vezes */
typedef struct {
    Rectangle corpo;
    bool ativo;
    bool contado;
} Obstaculo;

/* Estado completo da corrida: jogador, obstaculos, velocidade, pontos e chao */
typedef struct {
    Jogador jogador;
    Obstaculo obstaculos[MAX_OBSTACULOS];
    float velocidade;
    float tempoSpawn;
    float pontos;
    float deslocamentoChao;
} Jogo;

/* ---------- Fase de puzzle ---------- */

typedef struct {
    Rectangle corpo;
    Vector2 direcaoOlhar; /* ultima direcao de movimento, usada para mirar */
} JogadorPuzzle;

typedef struct {
    Rectangle corpo;
    bool quebravel; /* true = destrutivel a tiro, false = parede fixa */
    bool ativo;
} Bloco;

typedef struct {
    Rectangle corpo;
    Vector2 direcao;
    bool ativo;
} Bala;

typedef struct {
    JogadorPuzzle jogador;
    Bloco blocos[MAX_BLOCOS];
    int numBlocos;
    Bala balas[MAX_BALAS];
    int balasRestantes;
    Rectangle saida;
} Puzzle;

/* ---------- Estado do minigame (fica guardado entre os quadros) ---------- */

/* Tela atual do minigame */
static Tela tela = TELA_JOGO;
/* Estado da corrida de obstaculos */
static Jogo jogo;
/* Estado da fase de puzzle atual */
static Puzzle puzzle;
/* Numero da fase do puzzle (1 a TOTAL_FASES) */
static int faseAtual = 0;
/* Melhor pontuacao ja feita (lida de recorde.dat) */
static int recorde = 0;
/* Pontos da ultima corrida, mostrados na tela de fim */
static int pontuacaoFinal = 0;
/* true se a ultima corrida bateu o recorde */
static bool novoRecorde = false;

/* ---------- Recorde ---------- */

/* Le o recorde do arquivo; devolve 0 se o arquivo nao existir */
static int CarregarRecorde(void)
{
    int valor = 0;
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "rb");
    if (arquivo != NULL) {
        if (fread(&valor, sizeof(int), 1, arquivo) != 1) valor = 0;
        fclose(arquivo);
    }
    return valor;
}

/* Grava o recorde no arquivo */
static void SalvarRecorde(int valor)
{
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "wb");
    if (arquivo != NULL) {
        fwrite(&valor, sizeof(int), 1, arquivo);
        fclose(arquivo);
    }
}

/* ---------- Logica da corrida ---------- */

/* Volta a corrida ao estado inicial (pontos zerados, sem obstaculos) */
static void ReiniciarJogo(Jogo *partida)
{
    partida->jogador.corpo = (Rectangle){ JOG_X, CHAO_Y - JOG_ALTURA, JOG_LARGURA, JOG_ALTURA };
    partida->jogador.velY = 0.0f;
    partida->jogador.noChao = true;

    for (int i = 0; i < MAX_OBSTACULOS; i++) partida->obstaculos[i].ativo = false;

    partida->velocidade = VEL_INICIAL;
    partida->tempoSpawn = 1.2f;
    partida->pontos = 0.0f;
    partida->deslocamentoChao = 0.0f;
}

/* Coloca um novo obstaculo, de tamanho aleatorio, na borda direita da tela */
static void CriarObstaculo(Jogo *partida)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        if (!partida->obstaculos[i].ativo) {
            float largura = (float)GetRandomValue(24, 50);
            float altura = (float)GetRandomValue(40, 90);
            partida->obstaculos[i].corpo = (Rectangle){ LARGURA + 20.0f, CHAO_Y - altura, largura, altura };
            partida->obstaculos[i].ativo = true;
            partida->obstaculos[i].contado = false;
            return;
        }
    }
}

/* Retorna o resultado do frame: colisao, transicao para o modo puzzle, ou nada. */
static ResultadoCorrida AtualizarJogo(Jogo *partida, float tempoQuadro)
{
    Jogador *jogador = &partida->jogador;

    bool pediuPulo = IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W);
    if (pediuPulo && jogador->noChao) {
        jogador->velY = -FORCA_PULO;
        jogador->noChao = false;
    }

    jogador->velY += GRAVIDADE * tempoQuadro;
    jogador->corpo.y += jogador->velY * tempoQuadro;
    if (jogador->corpo.y >= CHAO_Y - jogador->corpo.height) {
        jogador->corpo.y = CHAO_Y - jogador->corpo.height;
        jogador->velY = 0.0f;
        jogador->noChao = true;
    }

    partida->velocidade += ACELERACAO * tempoQuadro;
    if (partida->velocidade > VEL_MAXIMA) partida->velocidade = VEL_MAXIMA;

    partida->pontos += PONTOS_POR_SEG * tempoQuadro;
    partida->deslocamentoChao += partida->velocidade * tempoQuadro;
    if (partida->deslocamentoChao >= 60.0f) partida->deslocamentoChao -= 60.0f;

    partida->tempoSpawn -= tempoQuadro;
    if (partida->tempoSpawn <= 0.0f) {
        CriarObstaculo(partida);
        partida->tempoSpawn = 1.0f + (float)GetRandomValue(0, 100) / 100.0f;
    }

    Rectangle hitbox = { jogador->corpo.x + 6, jogador->corpo.y + 6, jogador->corpo.width - 12, jogador->corpo.height - 8 };

    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        Obstaculo *obstaculo = &partida->obstaculos[i];
        if (!obstaculo->ativo) continue;

        obstaculo->corpo.x -= partida->velocidade * tempoQuadro;

        if (obstaculo->corpo.x + obstaculo->corpo.width < 0.0f) {
            obstaculo->ativo = false;
            continue;
        }

        if (!obstaculo->contado && obstaculo->corpo.x + obstaculo->corpo.width < jogador->corpo.x) {
            obstaculo->contado = true;
            partida->pontos += BONUS_OBSTACULO;
        }

        if (CheckCollisionRecs(hitbox, obstaculo->corpo)) return RESULTADO_COLISAO;
    }

    if (partida->pontos >= PONTOS_TRANSICAO) return RESULTADO_TRANSICAO;

    return RESULTADO_NADA;
}

/* ---------- Desenho da corrida ---------- */

/* Escreve um texto centralizado na largura do minigame */
static void TextoCentral(const char *texto, int y, int tamanho, Color cor)
{
    DrawText(texto, (LARGURA - MeasureText(texto, tamanho)) / 2, y, tamanho, cor);
}

/* Desenha o fundo e o chao, que "anda" conforme o deslocamento */
static void DesenharCenario(float deslocamento)
{
    ClearBackground((Color){ 24, 24, 38, 255 });
    DrawRectangle(0, CHAO_Y, LARGURA, ALTURA - CHAO_Y, (Color){ 46, 46, 70, 255 });
    DrawRectangle(0, CHAO_Y, LARGURA, 3, RAYWHITE);

    int inicio = -(int)deslocamento;
    for (int x = inicio; x < LARGURA; x += 60) {
        DrawRectangle(x, CHAO_Y + 25, 24, 4, (Color){ 90, 90, 120, 255 });
    }
}

/* Desenha os obstaculos e o jogador */
static void DesenharObjetos(const Jogo *partida)
{
    for (int i = 0; i < MAX_OBSTACULOS; i++) {
        const Obstaculo *obstaculo = &partida->obstaculos[i];
        if (!obstaculo->ativo) continue;
        DrawRectangleRec(obstaculo->corpo, (Color){ 230, 80, 80, 255 });
        DrawRectangleLinesEx(obstaculo->corpo, 2, MAROON);
    }

    const Rectangle *corpoJogador = &partida->jogador.corpo;
    DrawRectangleRec(*corpoJogador, SKYBLUE);
    DrawRectangleLinesEx(*corpoJogador, 2, BLUE);
    DrawRectangle((int)corpoJogador->x + 24, (int)corpoJogador->y + 10, 10, 10, WHITE);
    DrawRectangle((int)corpoJogador->x + 29, (int)corpoJogador->y + 13, 5, 5, BLACK);
}

/* Desenha os pontos (esquerda) e o recorde (direita) */
static void DesenharHud(const Jogo *partida, int melhor)
{
    DrawText(TextFormat("Pontos: %d", (int)partida->pontos), 20, 16, 24, RAYWHITE);

    const char *textoRecorde = TextFormat("Recorde: %d", melhor);
    DrawText(textoRecorde, LARGURA - MeasureText(textoRecorde, 24) - 20, 16, 24, GOLD);
}

/* Tela de fim de jogo: pontos, recorde e opcoes */
static void DesenharFim(int pontuacao, int melhor, bool ehNovoRecorde)
{
    DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.6f));
    TextoCentral("FIM DE JOGO", 100, 50, RAYWHITE);
    TextoCentral(TextFormat("Pontos: %d", pontuacao), 180, 30, RAYWHITE);

    if (ehNovoRecorde) {
        TextoCentral("Novo recorde!", 225, 26, GOLD);
    } else {
        TextoCentral(TextFormat("Recorde: %d", melhor), 225, 26, LIGHTGRAY);
    }

    TextoCentral("ENTER: jogar de novo    ESC: menu", 300, 20, LIGHTGRAY);
}

/* ---------- Logica do puzzle ---------- */

/* Adiciona um bloco a sala do puzzle */
static void AdicionarBloco(Puzzle *sala, float x, float y, float largura, float altura, bool quebravel)
{
    sala->blocos[sala->numBlocos].corpo = (Rectangle){ x, y, largura, altura };
    sala->blocos[sala->numBlocos].quebravel = quebravel;
    sala->blocos[sala->numBlocos].ativo = true;
    sala->numBlocos++;
}

/* Monta a sala de cada fase: paredes fixas (quebravel=false) e blocos
   destrutiveis (quebravel=true) que so cedem a tiro. */
static void CarregarFase(int fase, Puzzle *sala)
{
    sala->numBlocos = 0;
    for (int i = 0; i < MAX_BALAS; i++) sala->balas[i].ativo = false;
    sala->balasRestantes = BALAS_POR_FASE;
    sala->jogador.direcaoOlhar = (Vector2){ 1.0f, 0.0f };
    sala->jogador.corpo = (Rectangle){ 60.0f, 150.0f, 30.0f, 30.0f };
    sala->saida = (Rectangle){ 740.0f, 170.0f, 20.0f, 90.0f };

    switch (fase) {
    case 1:
        AdicionarBloco(sala, 250, 90, 20, 180, false);
        AdicionarBloco(sala, 500, 90, 40, 300, true);
        break;

    case 2:
        AdicionarBloco(sala, 220, 90, 20, 210, false);
        AdicionarBloco(sala, 420, 180, 20, 210, false);
        AdicionarBloco(sala, 470, 90, 40, 300, true);
        AdicionarBloco(sala, 650, 90, 40, 300, true);
        break;

    case 3:
    default:
        AdicionarBloco(sala, 150, 90, 20, 210, false);
        AdicionarBloco(sala, 190, 90, 40, 300, true);
        AdicionarBloco(sala, 350, 180, 20, 210, false);
        AdicionarBloco(sala, 390, 90, 40, 300, true);
        AdicionarBloco(sala, 550, 90, 20, 210, false);
        AdicionarBloco(sala, 590, 90, 40, 300, true);
        break;
    }
}

/* true se o retangulo "alvo" bate em algum bloco ainda ativo */
static bool ColideComSolidos(const Puzzle *sala, Rectangle alvo)
{
    for (int i = 0; i < sala->numBlocos; i++) {
        if (sala->blocos[i].ativo && CheckCollisionRecs(alvo, sala->blocos[i].corpo)) return true;
    }
    return false;
}

/* Move o jogador, trata os tiros e confere se chegou na saida */
static ResultadoPuzzle AtualizarPuzzle(Puzzle *sala, float tempoQuadro)
{
    JogadorPuzzle *jogador = &sala->jogador;
    Vector2 movimento = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) movimento.x += 1.0f;
    if (IsKeyDown(KEY_LEFT)  || IsKeyDown(KEY_A)) movimento.x -= 1.0f;
    if (IsKeyDown(KEY_DOWN)  || IsKeyDown(KEY_S)) movimento.y += 1.0f;
    if (IsKeyDown(KEY_UP)    || IsKeyDown(KEY_W)) movimento.y -= 1.0f;

    if (movimento.x != 0.0f || movimento.y != 0.0f) {
        float comprimento = sqrtf(movimento.x * movimento.x + movimento.y * movimento.y);
        movimento.x /= comprimento;
        movimento.y /= comprimento;
        jogador->direcaoOlhar = movimento; /* mira sempre na ultima direcao andada */
    }

    Rectangle tentativaX = jogador->corpo;
    tentativaX.x += movimento.x * PUZZLE_VEL * tempoQuadro;
    if (tentativaX.x >= PUZZLE_AREA_X &&
        tentativaX.x + tentativaX.width <= PUZZLE_AREA_X + PUZZLE_AREA_LARGURA &&
        !ColideComSolidos(sala, tentativaX)) {
        jogador->corpo.x = tentativaX.x;
    }

    Rectangle tentativaY = jogador->corpo;
    tentativaY.y += movimento.y * PUZZLE_VEL * tempoQuadro;
    if (tentativaY.y >= PUZZLE_AREA_Y &&
        tentativaY.y + tentativaY.height <= PUZZLE_AREA_Y + PUZZLE_AREA_ALTURA &&
        !ColideComSolidos(sala, tentativaY)) {
        jogador->corpo.y = tentativaY.y;
    }

    if (IsKeyPressed(KEY_SPACE) && sala->balasRestantes > 0) {
        for (int i = 0; i < MAX_BALAS; i++) {
            if (!sala->balas[i].ativo) {
                float cx = jogador->corpo.x + jogador->corpo.width / 2.0f - 5.0f;
                float cy = jogador->corpo.y + jogador->corpo.height / 2.0f - 5.0f;
                sala->balas[i].corpo = (Rectangle){ cx, cy, 10.0f, 10.0f };
                sala->balas[i].direcao = jogador->direcaoOlhar;
                sala->balas[i].ativo = true;
                sala->balasRestantes--;
                break;
            }
        }
    }

    for (int i = 0; i < MAX_BALAS; i++) {
        Bala *bala = &sala->balas[i];
        if (!bala->ativo) continue;

        bala->corpo.x += bala->direcao.x * BALA_VEL * tempoQuadro;
        bala->corpo.y += bala->direcao.y * BALA_VEL * tempoQuadro;

        if (bala->corpo.x < PUZZLE_AREA_X || bala->corpo.x > PUZZLE_AREA_X + PUZZLE_AREA_LARGURA ||
            bala->corpo.y < PUZZLE_AREA_Y || bala->corpo.y > PUZZLE_AREA_Y + PUZZLE_AREA_ALTURA) {
            bala->ativo = false;
            continue;
        }

        for (int k = 0; k < sala->numBlocos; k++) {
            Bloco *bloco = &sala->blocos[k];
            if (!bloco->ativo) continue;
            if (CheckCollisionRecs(bala->corpo, bloco->corpo)) {
                bala->ativo = false;
                if (bloco->quebravel) bloco->ativo = false;
                break;
            }
        }
    }

    if (CheckCollisionRecs(jogador->corpo, sala->saida)) return PUZZLE_SAIU;
    return PUZZLE_NADA;
}

/* ---------- Desenho do puzzle ---------- */

/* Desenha a sala do puzzle, a saida, as balas e o jogador */
static void DesenharPuzzle(const Puzzle *sala, int fase)
{
    ClearBackground((Color){ 18, 18, 30, 255 });
    DrawRectangleLines(PUZZLE_AREA_X, PUZZLE_AREA_Y, PUZZLE_AREA_LARGURA, PUZZLE_AREA_ALTURA, RAYWHITE);

    for (int i = 0; i < sala->numBlocos; i++) {
        const Bloco *bloco = &sala->blocos[i];
        if (!bloco->ativo) continue;
        Color cor = bloco->quebravel ? (Color){ 200, 140, 60, 255 } : (Color){ 90, 90, 110, 255 };
        DrawRectangleRec(bloco->corpo, cor);
        DrawRectangleLinesEx(bloco->corpo, 2, bloco->quebravel ? MAROON : BLACK);
    }

    DrawRectangleRec(sala->saida, (Color){ 60, 200, 100, 255 });
    DrawText("SAIDA", (int)sala->saida.x - 12, (int)sala->saida.y - 22, 16, GREEN);

    for (int i = 0; i < MAX_BALAS; i++) {
        if (sala->balas[i].ativo) DrawRectangleRec(sala->balas[i].corpo, YELLOW);
    }

    const Rectangle *corpoJogador = &sala->jogador.corpo;
    DrawRectangleRec(*corpoJogador, SKYBLUE);
    DrawRectangleLinesEx(*corpoJogador, 2, BLUE);
    float cx = corpoJogador->x + corpoJogador->width / 2.0f;
    float cy = corpoJogador->y + corpoJogador->height / 2.0f;
    DrawLine((int)cx, (int)cy,
             (int)(cx + sala->jogador.direcaoOlhar.x * 20.0f),
             (int)(cy + sala->jogador.direcaoOlhar.y * 20.0f), WHITE);

    DrawText(TextFormat("Fase %d/%d", fase, TOTAL_FASES), 20, 16, 24, RAYWHITE);
    DrawText(TextFormat("Balas: %d", sala->balasRestantes), LARGURA - 140, 16, 24, YELLOW);
    DrawText("WASD/setas: mover   ESPACO: atirar   R: reiniciar fase   ESC: menu",
             20, ALTURA - 26, 16, LIGHTGRAY);
}

/* =====================================================
   FUNCOES PUBLICAS (usadas pelo game.c)
   ===================================================== */

/* Prepara o minigame para comecar do zero */
void InitMinigame(void)
{
    SetRandomSeed((unsigned int)time(NULL));

    recorde = CarregarRecorde();
    pontuacaoFinal = 0;
    novoRecorde = false;
    faseAtual = 0;

    ReiniciarJogo(&jogo);

    tela = TELA_JOGO;
}


/* Atualiza o minigame e informa se continua, volta ao menu ou foi vencido */
MinigameResult UpdateMinigame(void)
{
    float tempoQuadro = GetFrameTime();
    if (tempoQuadro > 0.05f) tempoQuadro = 0.05f;

    switch (tela) {

    case TELA_JOGO: {
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;

        ResultadoCorrida resultado = AtualizarJogo(&jogo, tempoQuadro);

        if (resultado == RESULTADO_COLISAO) {
            pontuacaoFinal = (int)jogo.pontos;
            novoRecorde = pontuacaoFinal > recorde;
            if (novoRecorde) {
                recorde = pontuacaoFinal;
                SalvarRecorde(recorde);
            }
            tela = TELA_FIM;
        } else if (resultado == RESULTADO_TRANSICAO) {
            faseAtual = 1;
            CarregarFase(faseAtual, &puzzle);
            tela = TELA_PUZZLE;
        }
        break;
    }

    case TELA_FIM:
        if (IsKeyPressed(KEY_ENTER)) {
            ReiniciarJogo(&jogo);
            tela = TELA_JOGO;
        }
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;

    case TELA_PUZZLE:
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;

        if (IsKeyPressed(KEY_R)) {
            CarregarFase(faseAtual, &puzzle);
            break;
        }

        if (AtualizarPuzzle(&puzzle, tempoQuadro) == PUZZLE_SAIU) {
            faseAtual++;
            if (faseAtual > TOTAL_FASES) {
                tela = TELA_VITORIA;
            } else {
                CarregarFase(faseAtual, &puzzle);
            }
        }
        break;

    case TELA_VITORIA:
        if (IsKeyPressed(KEY_ENTER)) return MINIGAME_VENCEU;
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;
    }

    return MINIGAME_CONTINUA;
}


/* Desenha o minigame ampliado (camera) para caber na janela do jogo */
void DrawMinigame(void)
{
    // O minigame foi desenhado em 800x450: a camera amplia para a janela.
    float zoomX = (float)GetScreenWidth() / (float)LARGURA;
    float zoomY = (float)GetScreenHeight() / (float)ALTURA;

    Camera2D camera = { 0 };
    camera.zoom = (zoomX < zoomY) ? zoomX : zoomY;

    BeginMode2D(camera);

    switch (tela) {

    case TELA_JOGO:
        DesenharCenario(jogo.deslocamentoChao);
        DesenharObjetos(&jogo);
        DesenharHud(&jogo, recorde);
        break;

    case TELA_FIM:
        DesenharCenario(jogo.deslocamentoChao);
        DesenharObjetos(&jogo);
        DesenharHud(&jogo, recorde);
        DesenharFim(pontuacaoFinal, recorde, novoRecorde);
        break;

    case TELA_PUZZLE:
        DesenharPuzzle(&puzzle, faseAtual);
        break;

    case TELA_VITORIA:
        ClearBackground((Color){ 18, 18, 30, 255 });
        TextoCentral("REDE RESTAURADA!", 160, 40, GOLD);
        TextoCentral("Voce completou as 3 fases.", 220, 24, RAYWHITE);
        TextoCentral("ENTER: continuar    ESC: menu", 290, 20, LIGHTGRAY);
        break;
    }

    EndMode2D();
}