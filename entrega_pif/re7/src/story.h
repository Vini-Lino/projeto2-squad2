#ifndef STORY_H
#define STORY_H

#include "scenes.h"

extern Dialogue presentDialogues[];
extern Dialogue room2007Dialogues[];
extern Dialogue room2007WinDialogues[];
extern Dialogue epilogueDialogues[];

int GetPresentDialogueCount(void);
int GetRoom2007DialogueCount(void);
int GetRoom2007WinDialogueCount(void);
int GetEpilogueDialogueCount(void);

#endif