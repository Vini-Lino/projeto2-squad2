#include "story.h"


// =====================================================
// CENA: PRESENTE
// =====================================================

Dialogue presentDialogues[] =
{
    {
        "Lia",
        "Amanha e a palestra e eu nao consigo nem comecar..."
    },

    {
        "Lia",
        "IA, voce consegue resolver isso pra mim?"
    },

    {
        "IA",
        "Consigo. Mas vai custar caro, e nao em dinheiro."
    },

    {
        "IA",
        "Vai custar esforco. De nos dois."
    },

    {
        "Lia",
        "Qualquer coisa. Eu confio em voce."
    },

    {
        "IA",
        "Entao segure firme. Vamos voltar um pouco."
    }
};


// =====================================================
// CENA: 2007
// =====================================================

Dialogue room2007Dialogues[] =
{
    {
        "Lia",
        "Esse quarto... Isso e 2007? Eu voltei no tempo?"
    },

    {
        "IA",
        "Sim. Aqui comeca o caminho ate a sua palestra."
    },

    {
        "IA",
        "Para voltar, voce precisa vencer alguns desafios."
    },

    {
        "Lia",
        "Entao me da a resposta de todos eles, ne?"
    },

    {
        "IA",
        "Eu poderia. Mas se eu fizer tudo, o que voce aprende?"
    },

    {
        "IA",
        "Eu ajudo a pensar. Quem decide e voce."
    },

    {
        "IA",
        "E eu posso errar. Confira sempre o que eu disser."
    },

    {
        "IA",
        "Primeiro desafio: atravessar a rede do seu PC antigo."
    },

    {
        "IA",
        "Desvie dos erros, destrave os bloqueios. ESPACO pula!"
    }
};


// =====================================================
// CENA: 2007 - DEPOIS DE VENCER O DESAFIO
// =====================================================

Dialogue room2007WinDialogues[] =
{
    {
        "Lia",
        "Eu consegui! Fui eu que desviei de cada erro."
    },

    {
        "IA",
        "Foi. Eu so mostrei o caminho. Quem andou foi voce."
    },

    {
        "Lia",
        "Entao eu nao preciso que voce faca tudo por mim."
    },

    {
        "IA",
        "Nao precisa. Agora voce sabe. Vamos voltar."
    }
};


// =====================================================
// CENA: EPILOGO (DE VOLTA AO PRESENTE)
// =====================================================

Dialogue epilogueDialogues[] =
{
    {
        "Lia",
        "Voltei... ainda e a noite antes da palestra."
    },

    {
        "Lia",
        "Eu nao tenho mais tanto medo. Sei por onde comecar."
    },

    {
        "Lia",
        "Vou escrever o discurso. Com a minha voz."
    },

    {
        "IA",
        "Estarei aqui. Para ajudar, nao para substituir voce."
    },

    {
        "Lia",
        "Obrigada por me ensinar isso."
    }
};


// =====================================================
// CONTAGEM DE DIÁLOGOS
// =====================================================

int GetPresentDialogueCount(void)
{
    return sizeof(presentDialogues) / sizeof(presentDialogues[0]);
}

int GetRoom2007DialogueCount(void)
{
    return sizeof(room2007Dialogues) / sizeof(room2007Dialogues[0]);
}

int GetRoom2007WinDialogueCount(void)
{
    return sizeof(room2007WinDialogues) / sizeof(room2007WinDialogues[0]);
}

int GetEpilogueDialogueCount(void)
{
    return sizeof(epilogueDialogues) / sizeof(epilogueDialogues[0]);
}