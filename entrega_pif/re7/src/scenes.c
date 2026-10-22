#include "scenes.h"
#include "story.h"


// =====================================================
// RETORNAR CENA
// =====================================================

Scene GetScene(SceneID id)
{
    switch (id)
    {

        case SCENE_PRESENT:
        {
            Scene scene =
            {
                presentDialogues,
                GetPresentDialogueCount(),
                SCENE_2007
            };

            return scene;
        }


        case SCENE_2007:
        {
            Scene scene =
            {
                room2007Dialogues,
                GetRoom2007DialogueCount(),
                SCENE_PRESENT
            };

            return scene;
        }


        case SCENE_2007_CHALLENGE:
        {
            Scene scene =
            {
                room2007ChallengeDialogues,
                GetRoom2007ChallengeDialogueCount(),
                SCENE_2007_WIN
            };

            return scene;
        }


        case SCENE_2007_WIN:
        {
            Scene scene =
            {
                room2007WinDialogues,
                GetRoom2007WinDialogueCount(),
                SCENE_EPILOGUE
            };

            return scene;
        }


        case SCENE_EPILOGUE:
        {
            Scene scene =
            {
                epilogueDialogues,
                GetEpilogueDialogueCount(),
                SCENE_PRESENT
            };

            return scene;
        }


        default:
        {
            Scene empty = {0};

            return empty;
        }
    }
}