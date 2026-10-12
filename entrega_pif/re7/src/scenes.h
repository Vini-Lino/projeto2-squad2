#ifndef SCENES_H
#define SCENES_H


// Todas as cenas do jogo

typedef enum
{
    SCENE_PRESENT,
    SCENE_2007,
    SCENE_2007_WIN,
    SCENE_EPILOGUE,

    SCENE_COUNT

} SceneID;


// Uma fala

typedef struct
{
    const char *speaker;

    const char *text;

} Dialogue;


// Dados de uma cena

typedef struct
{
    Dialogue *dialogues;

    int dialogueCount;

    SceneID nextScene;

} Scene;


// Retorna os dados da cena

Scene GetScene(SceneID id);


#endif