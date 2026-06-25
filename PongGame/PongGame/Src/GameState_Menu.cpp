#include "Main.h"
#include "GameState_Menu.h"
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
extern "C" {
    EMSCRIPTEN_KEEPALIVE void StartGame() { gGameStateNext = GS_PONG; }
}
#endif

static bool sInputReady = false;

void GameState_MenuLoad()   {}

void GameState_MenuInit()
{
    sInputReady = false;  // skip first frame so held keys from prior state don't fire
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
#if defined(__EMSCRIPTEN__)
    EM_ASM(
        var btn = document.getElementById('btn-start');
        if (btn) btn.style.display = 'block';
    );
#endif
}

void GameState_MenuUpdate()
{
    if (!sInputReady) { sInputReady = true; return; }

    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        gGameStateNext = GS_QUIT;
        return;
    }
    if (AEInputCheckTriggered(AEVK_SPACE))
        gGameStateNext = GS_PONG;
}

void GameState_MenuDraw()
{
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxPrint(gFontId, "PONG",                -0.18f,  0.20f, 3.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxPrint(gFontId, "Press SPACE to Start", -0.35f, -0.10f, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
}

void GameState_MenuFree()
{
#if defined(__EMSCRIPTEN__)
    EM_ASM(
        var btn = document.getElementById('btn-start');
        if (btn) btn.style.display = 'none';
    );
#endif
}

void GameState_MenuUnload() {}
