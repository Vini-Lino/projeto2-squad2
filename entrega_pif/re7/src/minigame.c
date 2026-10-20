/*
 * Minigame da visual novel Re:7
 * Parte 1: voo vertical inspirado em jogos de arcade de desvio.
 * Parte 2: salas de puzzle solo com caixas, placas, chave e portas.
 *
 * Mantem a API publica usada por game.c: InitMinigame, UpdateMinigame,
 * DrawMinigame. Desenhado em 800x450 e ampliado para a janela do jogo.
 */

#include "minigame.h"
#include "raylib.h"

#include <math.h>
#include <stdio.h>
#include <time.h>

#define LARGURA 800
#define ALTURA 450
#define ARQUIVO_RECORDE "recorde_voo.dat"

/* Voo */
#define JOG_X 170.0f
#define JOG_LARGURA 34.0f
#define JOG_ALTURA 28.0f
#define GRAVIDADE 1250.0f
#define IMPULSO_VOO 430.0f
#define VELOCIDADE_INICIAL 245.0f
#define VELOCIDADE_MAXIMA 360.0f
#define MAX_PORTAIS 5
#define LARGURA_PORTAL 64.0f
#define ABERTURA_MINIMA 145.0f
#define PORTAIS_PARA_VENCER 9

/* Salas de puzzle */
#define TILE 40
#define GRID_COLS 18
#define GRID_ROWS 7
#define AREA_X 40
#define AREA_Y 90
#define MAX_PAREDES 32
#define MAX_CAIXAS 2
#define PASSO_JOGADOR 0.14f

typedef enum { TELA_VOO, TELA_FIM, TELA_PUZZLE, TELA_VITORIA } Tela;
typedef enum { VOO_NADA, VOO_COLISAO, VOO_TRANSICAO } ResultadoVoo;

typedef struct {
    float x;
    float aberturaY;
    float aberturaAltura;
    bool ativo;
    bool contado;
} Portal;

typedef struct {
    Rectangle jogador;
    float velocidadeY;
    float velocidadeMundo;
    float tempoSpawn;
    float tempo;
    int portaisPassados;
    float fundoOffset;
    Portal portais[MAX_PORTAIS];
} Voo;

typedef struct { int x, y; } TilePos;

typedef struct {
    TilePos jogador;
    TilePos caixas[MAX_CAIXAS];
    int quantidadeCaixas;
    TilePos paredes[MAX_PAREDES];
    int quantidadeParedes;
    TilePos placas[MAX_CAIXAS];
    int quantidadePlacas;
    TilePos chave;
    bool temChave;
    bool exigeChave;
    bool chaveColetada;
    TilePos saida;
    float tempoPasso;
} Puzzle;

static Tela tela = TELA_VOO;
static Voo voo;
static Puzzle puzzle;
static int faseAtual = 1;
static int recorde = 0;
static int pontuacaoFinal = 0;
static bool novoRecorde = false;

static int CarregarRecorde(void)
{
    int valor = 0;
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "rb");
    if (arquivo != NULL) {
        if (fread(&valor, sizeof(valor), 1, arquivo) != 1) valor = 0;
        fclose(arquivo);
    }
    return valor;
}

static void SalvarRecorde(int valor)
{
    FILE *arquivo = fopen(ARQUIVO_RECORDE, "wb");
    if (arquivo != NULL) {
        fwrite(&valor, sizeof(valor), 1, arquivo);
        fclose(arquivo);
    }
}

static void ReiniciarVoo(void)
{
    voo.jogador = (Rectangle){ JOG_X, ALTURA * 0.5f, JOG_LARGURA, JOG_ALTURA };
    voo.velocidadeY = 0.0f;
    voo.velocidadeMundo = VELOCIDADE_INICIAL;
    voo.tempoSpawn = 0.7f;
    voo.tempo = 0.0f;
    voo.portaisPassados = 0;
    voo.fundoOffset = 0.0f;
    for (int i = 0; i < MAX_PORTAIS; i++) voo.portais[i].ativo = false;
}

static void CriarPortal(void)
{
    for (int i = 0; i < MAX_PORTAIS; i++) {
        if (voo.portais[i].ativo) continue;
        float minimo = 90.0f;
        float maximo = ALTURA - 55.0f - ABERTURA_MINIMA;
        voo.portais[i].x = LARGURA + 20.0f;
        voo.portais[i].aberturaY = (float)GetRandomValue((int)minimo, (int)maximo);
        voo.portais[i].aberturaAltura = ABERTURA_MINIMA;
        voo.portais[i].ativo = true;
        voo.portais[i].contado = false;
        return;
    }
}

static ResultadoVoo AtualizarVoo(float dt)
{
    if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        voo.velocidadeY = -IMPULSO_VOO;
    }

    voo.velocidadeY += GRAVIDADE * dt;
    voo.jogador.y += voo.velocidadeY * dt;
    voo.tempo += dt;
    voo.velocidadeMundo += 4.0f * dt;
    if (voo.velocidadeMundo > VELOCIDADE_MAXIMA) voo.velocidadeMundo = VELOCIDADE_MAXIMA;
    voo.fundoOffset += voo.velocidadeMundo * 0.28f * dt;
    if (voo.fundoOffset >= 80.0f) voo.fundoOffset -= 80.0f;

    if (voo.jogador.y < 48.0f || voo.jogador.y + voo.jogador.height > ALTURA - 18.0f) {
        return VOO_COLISAO;
    }

    voo.tempoSpawn -= dt;
    if (voo.tempoSpawn <= 0.0f) {
        CriarPortal();
        voo.tempoSpawn = 1.55f;
    }

    Rectangle hitbox = { voo.jogador.x + 5, voo.jogador.y + 4,
                         voo.jogador.width - 10, voo.jogador.height - 8 };
    for (int i = 0; i < MAX_PORTAIS; i++) {
        Portal *portal = &voo.portais[i];
        if (!portal->ativo) continue;
        portal->x -= voo.velocidadeMundo * dt;
        Rectangle topo = { portal->x, 0, LARGURA_PORTAL, portal->aberturaY };
        float inicioBaixo = portal->aberturaY + portal->aberturaAltura;
        Rectangle baixo = { portal->x, inicioBaixo, LARGURA_PORTAL, ALTURA - inicioBaixo };
        if (CheckCollisionRecs(hitbox, topo) || CheckCollisionRecs(hitbox, baixo)) return VOO_COLISAO;
        if (!portal->contado && portal->x + LARGURA_PORTAL < voo.jogador.x) {
            portal->contado = true;
            voo.portaisPassados++;
        }
        if (portal->x + LARGURA_PORTAL < 0) portal->ativo = false;
    }

    if (voo.portaisPassados >= PORTAIS_PARA_VENCER) return VOO_TRANSICAO;
    return VOO_NADA;
}

static void DesenharVoo(void)
{
    ClearBackground((Color){ 18, 28, 55, 255 });
    /* Fundo em movimento */
    for (int i = 0; i < 12; i++) {
        int x = (int)(i * 80 - voo.fundoOffset);
        DrawCircle(x, 65 + (i % 3) * 42, 2, (Color){ 150, 190, 230, 180 });
        DrawCircle(x + 35, 185 + (i % 4) * 35, 2, (Color){ 150, 190, 230, 150 });
    }
    DrawRectangle(0, ALTURA - 18, LARGURA, 18, (Color){ 37, 47, 73, 255 });
    DrawRectangle(0, ALTURA - 18, LARGURA, 3, (Color){ 110, 180, 220, 255 });

    for (int i = 0; i < MAX_PORTAIS; i++) {
        const Portal *p = &voo.portais[i];
        if (!p->ativo) continue;
        float baixoY = p->aberturaY + p->aberturaAltura;
        Rectangle topo = { p->x, 0, LARGURA_PORTAL, p->aberturaY };
        Rectangle baixo = { p->x, baixoY, LARGURA_PORTAL, ALTURA - baixoY };
        DrawRectangleRec(topo, (Color){ 55, 190, 135, 255 });
        DrawRectangleRec(baixo, (Color){ 55, 190, 135, 255 });
        DrawRectangleLinesEx(topo, 3, (Color){ 20, 105, 93, 255 });
        DrawRectangleLinesEx(baixo, 3, (Color){ 20, 105, 93, 255 });
        DrawRectangle((int)p->x - 5, (int)p->aberturaY - 14, (int)LARGURA_PORTAL + 10, 14,
                      (Color){ 100, 225, 165, 255 });
        DrawRectangle((int)p->x - 5, (int)baixoY, (int)LARGURA_PORTAL + 10, 14,
                      (Color){ 100, 225, 165, 255 });
    }

    /* Personagem com pequena asa animada pelo movimento vertical */
    DrawRectangleRec(voo.jogador, SKYBLUE);
    DrawRectangleLinesEx(voo.jogador, 2, BLUE);
    DrawCircle((int)(voo.jogador.x + 25), (int)(voo.jogador.y + 9), 3, WHITE);
    DrawCircle((int)(voo.jogador.x + 26), (int)(voo.jogador.y + 9), 1.5f, BLACK);
    DrawTriangle((Vector2){voo.jogador.x + 3, voo.jogador.y + 15},
                 (Vector2){voo.jogador.x - 8, voo.jogador.y + 8 + voo.velocidadeY * 0.015f},
                 (Vector2){voo.jogador.x + 3, voo.jogador.y + 23}, ORANGE);

    DrawText(TextFormat("PORTAIS: %d/%d", voo.portaisPassados, PORTAIS_PARA_VENCER), 20, 16, 22, RAYWHITE);
    DrawText("ESPACO / W / ↑: impulsionar", 20, ALTURA - 42, 17, LIGHTGRAY);
    DrawText("Mantenha-se no ar e atravesse as aberturas", 335, ALTURA - 42, 16, LIGHTGRAY);
}

/* ---------- Puzzles em grade ---------- */

static void AdicionarParede(int x, int y)
{
    if (puzzle.quantidadeParedes >= MAX_PAREDES) return;
    puzzle.paredes[puzzle.quantidadeParedes++] = (TilePos){ x, y };
}

static void AdicionarCaixa(int x, int y)
{
    if (puzzle.quantidadeCaixas >= MAX_CAIXAS) return;
    puzzle.caixas[puzzle.quantidadeCaixas++] = (TilePos){ x, y };
}

static void CarregarFase(int fase)
{
    puzzle.quantidadeParedes = 0;
    puzzle.quantidadeCaixas = 0;
    puzzle.quantidadePlacas = 0;
    puzzle.chaveColetada = false;
    puzzle.exigeChave = false;
    puzzle.temChave = false;
    puzzle.tempoPasso = 0.0f;
    puzzle.jogador = (TilePos){ 2, 3 };
    puzzle.saida = (TilePos){ 16, 3 };

    if (fase == 1) {
        AdicionarCaixa(6, 3);
        puzzle.placas[puzzle.quantidadePlacas++] = (TilePos){ 11, 3 };
        /* Trilhos nas bordas deixam claro que a caixa precisa chegar à placa. */
        for (int x = 4; x <= 12; x++) { AdicionarParede(x, 1); AdicionarParede(x, 5); }
    } else if (fase == 2) {
        AdicionarCaixa(5, 3);
        puzzle.placas[puzzle.quantidadePlacas++] = (TilePos){ 12, 3 };
        puzzle.temChave = true;
        puzzle.exigeChave = true;
        puzzle.chave = (TilePos){ 9, 2 };
        /* Pilares formam um caminho em zigue-zague entre chave, caixa e saída. */
        /* Os dois vãos no primeiro pilar permitem contornar a caixa por cima. */
        for (int y = 1; y <= 4; y++) if (y == 3) AdicionarParede(8, y);
        for (int y = 2; y <= 5; y++) if (y != 4) AdicionarParede(10, y);
        for (int x = 11; x <= 14; x++) AdicionarParede(x, 1);
    } else {
        AdicionarCaixa(4, 2);
        AdicionarCaixa(4, 4);
        puzzle.placas[puzzle.quantidadePlacas++] = (TilePos){ 12, 2 };
        puzzle.placas[puzzle.quantidadePlacas++] = (TilePos){ 12, 4 };
        /* Duas passagens estreitas forçam a organizar as caixas separadamente. */
        for (int y = 1; y <= 5; y++) if (y != 2 && y != 4) AdicionarParede(8, y);
        for (int y = 1; y <= 5; y++) if (y != 2 && y != 4) AdicionarParede(10, y);
        for (int x = 11; x <= 14; x++) { AdicionarParede(x, 1); AdicionarParede(x, 5); }
    }
}

static bool MesmaPos(TilePos a, TilePos b) { return a.x == b.x && a.y == b.y; }

static bool TemParede(TilePos pos)
{
    if (pos.x < 1 || pos.x >= GRID_COLS - 1 || pos.y < 1 || pos.y >= GRID_ROWS - 1) return true;
    for (int i = 0; i < puzzle.quantidadeParedes; i++) if (MesmaPos(pos, puzzle.paredes[i])) return true;
    return false;
}

static int CaixaNaPos(TilePos pos)
{
    for (int i = 0; i < puzzle.quantidadeCaixas; i++) if (MesmaPos(pos, puzzle.caixas[i])) return i;
    return -1;
}

static bool PlacaAtivada(int indice)
{
    for (int i = 0; i < puzzle.quantidadeCaixas; i++)
        if (MesmaPos(puzzle.caixas[i], puzzle.placas[indice])) return true;
    return false;
}

static bool TodasPlacasAtivadas(void)
{
    for (int i = 0; i < puzzle.quantidadePlacas; i++) if (!PlacaAtivada(i)) return false;
    return true;
}

static bool PortaAberta(void)
{
    return TodasPlacasAtivadas() && (!puzzle.exigeChave || puzzle.chaveColetada);
}

static void TentarMover(int dx, int dy)
{
    TilePos destino = { puzzle.jogador.x + dx, puzzle.jogador.y + dy };
    if (TemParede(destino)) return;
    if (MesmaPos(destino, puzzle.saida) && !PortaAberta()) return;
    int caixa = CaixaNaPos(destino);
    if (caixa >= 0) {
        TilePos destinoCaixa = { destino.x + dx, destino.y + dy };
        if (TemParede(destinoCaixa) || CaixaNaPos(destinoCaixa) >= 0) return;
        puzzle.caixas[caixa] = destinoCaixa;
    }
    puzzle.jogador = destino;
    if (puzzle.temChave && MesmaPos(puzzle.jogador, puzzle.chave)) puzzle.chaveColetada = true;
}

static bool AtualizarPuzzle(float dt)
{
    puzzle.tempoPasso -= dt;
    if (IsKeyPressed(KEY_R)) { CarregarFase(faseAtual); return false; }
    if (puzzle.tempoPasso <= 0.0f) {
        int dx = 0, dy = 0;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) dx = 1;
        else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) dx = -1;
        else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) dy = 1;
        else if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) dy = -1;
        if (dx != 0 || dy != 0) {
            TentarMover(dx, dy);
            puzzle.tempoPasso = PASSO_JOGADOR;
        }
    }
    return PortaAberta() && MesmaPos(puzzle.jogador, puzzle.saida);
}

static Rectangle RetanguloTile(TilePos pos)
{
    return (Rectangle){ AREA_X + pos.x * TILE, AREA_Y + pos.y * TILE, TILE, TILE };
}

static void DesenharPuzzle(void)
{
    ClearBackground((Color){ 18, 20, 38, 255 });
    DrawText(TextFormat("FASE %d/3", faseAtual), 20, 16, 23, RAYWHITE);
    const char *objetivo = faseAtual == 1 ? "Empurre a caixa para a placa e atravesse a porta" :
                           faseAtual == 2 ? "Pegue a chave, ative a placa e abra a porta" :
                           "Coloque uma caixa em cada placa para abrir a saída";
    DrawText(objetivo, 20, 48, 17, LIGHTGRAY);

    DrawRectangle(AREA_X, AREA_Y, GRID_COLS * TILE, GRID_ROWS * TILE, (Color){ 30, 37, 58, 255 });
    for (int x = 0; x <= GRID_COLS; x++) DrawLine(AREA_X + x*TILE, AREA_Y, AREA_X + x*TILE, AREA_Y + GRID_ROWS*TILE, (Color){ 48, 57, 79, 255 });
    for (int y = 0; y <= GRID_ROWS; y++) DrawLine(AREA_X, AREA_Y + y*TILE, AREA_X + GRID_COLS*TILE, AREA_Y + y*TILE, (Color){ 48, 57, 79, 255 });

    for (int i = 0; i < puzzle.quantidadeParedes; i++) {
        Rectangle r = RetanguloTile(puzzle.paredes[i]);
        DrawRectangleRec(r, (Color){ 82, 92, 120, 255 });
        DrawRectangleLinesEx(r, 2, (Color){ 42, 49, 72, 255 });
    }
    for (int i = 0; i < puzzle.quantidadePlacas; i++) {
        Rectangle r = RetanguloTile(puzzle.placas[i]);
        bool ativa = PlacaAtivada(i);
        DrawRectangle((int)r.x + 5, (int)r.y + 5, TILE - 10, TILE - 10,
                      ativa ? (Color){ 70, 220, 145, 255 } : (Color){ 220, 170, 65, 255 });
        DrawText(ativa ? "ON" : "!", (int)r.x + 12, (int)r.y + 10, 18, BLACK);
    }
    if (puzzle.temChave && !puzzle.chaveColetada) {
        Rectangle r = RetanguloTile(puzzle.chave);
        DrawCircle((int)(r.x + TILE/2), (int)(r.y + TILE/2), 10, GOLD);
        DrawCircleLines((int)(r.x + TILE/2), (int)(r.y + TILE/2), 10, ORANGE);
    }
    for (int i = 0; i < puzzle.quantidadeCaixas; i++) {
        Rectangle r = RetanguloTile(puzzle.caixas[i]);
        DrawRectangle((int)r.x + 4, (int)r.y + 4, TILE - 8, TILE - 8, (Color){ 205, 135, 70, 255 });
        DrawRectangleLinesEx((Rectangle){r.x + 4, r.y + 4, TILE - 8, TILE - 8}, 2, BROWN);
        DrawLine((int)r.x + 11, (int)r.y + 11, (int)r.x + TILE - 11, (int)r.y + TILE - 11, BROWN);
        DrawLine((int)r.x + TILE - 11, (int)r.y + 11, (int)r.x + 11, (int)r.y + TILE - 11, BROWN);
    }

    bool portaAberta = PortaAberta();
    Rectangle saida = RetanguloTile(puzzle.saida);
    DrawRectangle((int)saida.x + 5, (int)saida.y + 3, TILE - 10, TILE - 6,
                  portaAberta ? (Color){ 70, 210, 130, 255 } : (Color){ 150, 65, 80, 255 });
    DrawText(portaAberta ? "GO" : "X", (int)saida.x + 10, (int)saida.y + 11, 18, RAYWHITE);

    Rectangle jogador = RetanguloTile(puzzle.jogador);
    DrawRectangle((int)jogador.x + 7, (int)jogador.y + 6, TILE - 14, TILE - 12, SKYBLUE);
    DrawRectangleLinesEx((Rectangle){jogador.x + 7, jogador.y + 6, TILE - 14, TILE - 12}, 2, BLUE);
    DrawCircle((int)jogador.x + 26, (int)jogador.y + 16, 2, WHITE);

    DrawText(TextFormat("Placas: %d/%d", (int)(PlacaAtivada(0) ? 1 : 0) +
             (puzzle.quantidadePlacas > 1 && PlacaAtivada(1) ? 1 : 0), puzzle.quantidadePlacas), 20, 390, 18, RAYWHITE);
    if (puzzle.exigeChave) DrawText(puzzle.chaveColetada ? "Chave: coletada" : "Chave: falta coletar", 190, 390, 18, GOLD);
    DrawText("WASD/setas: mover e empurrar   R: reiniciar fase   ESC: menu", 20, 420, 16, LIGHTGRAY);
}

static void TextoCentral(const char *texto, int y, int tamanho, Color cor)
{
    DrawText(texto, (LARGURA - MeasureText(texto, tamanho)) / 2, y, tamanho, cor);
}

void InitMinigame(void)
{
    SetRandomSeed((unsigned int)time(NULL));
    recorde = CarregarRecorde();
    pontuacaoFinal = 0;
    novoRecorde = false;
    faseAtual = 1;
    ReiniciarVoo();
    tela = TELA_VOO;
}

MinigameResult UpdateMinigame(void)
{
    float dt = GetFrameTime();
    if (dt > 0.05f) dt = 0.05f;
    switch (tela) {
    case TELA_VOO: {
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        ResultadoVoo resultado = AtualizarVoo(dt);
        if (resultado == VOO_COLISAO) {
            pontuacaoFinal = voo.portaisPassados;
            novoRecorde = pontuacaoFinal > recorde;
            if (novoRecorde) { recorde = pontuacaoFinal; SalvarRecorde(recorde); }
            tela = TELA_FIM;
        } else if (resultado == VOO_TRANSICAO) {
            faseAtual = 1;
            CarregarFase(faseAtual);
            tela = TELA_PUZZLE;
        }
        break;
    }
    case TELA_FIM:
        if (IsKeyPressed(KEY_ENTER)) { ReiniciarVoo(); tela = TELA_VOO; }
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;
    case TELA_PUZZLE:
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        if (AtualizarPuzzle(dt)) {
            faseAtual++;
            if (faseAtual > 3) tela = TELA_VITORIA;
            else CarregarFase(faseAtual);
        }
        break;
    case TELA_VITORIA:
        if (IsKeyPressed(KEY_ENTER)) return MINIGAME_VENCEU;
        if (IsKeyPressed(KEY_ESCAPE)) return MINIGAME_MENU;
        break;
    }
    return MINIGAME_CONTINUA;
}

void DrawMinigame(void)
{
    float zoomX = (float)GetScreenWidth() / LARGURA;
    float zoomY = (float)GetScreenHeight() / ALTURA;
    Camera2D camera = { 0 };
    camera.zoom = zoomX < zoomY ? zoomX : zoomY;
    BeginMode2D(camera);
    switch (tela) {
    case TELA_VOO:
        DesenharVoo();
        break;
    case TELA_FIM:
        DesenharVoo();
        DrawRectangle(0, 0, LARGURA, ALTURA, Fade(BLACK, 0.68f));
        TextoCentral("FIM DE JOGO", 115, 48, RAYWHITE);
        TextoCentral(TextFormat("Portais atravessados: %d", pontuacaoFinal), 190, 25, RAYWHITE);
        TextoCentral(novoRecorde ? "Novo recorde!" : TextFormat("Recorde: %d", recorde), 235, 23, GOLD);
        TextoCentral("ENTER: tentar de novo    ESC: menu", 300, 19, LIGHTGRAY);
        break;
    case TELA_PUZZLE:
        DesenharPuzzle();
        break;
    case TELA_VITORIA:
        ClearBackground((Color){ 18, 20, 38, 255 });
        TextoCentral("REDE RESTAURADA!", 145, 40, GOLD);
        TextoCentral("Voce completou as 3 salas de puzzle.", 215, 23, RAYWHITE);
        TextoCentral("ENTER: continuar    ESC: menu", 290, 19, LIGHTGRAY);
        break;
    }
    EndMode2D();
}
