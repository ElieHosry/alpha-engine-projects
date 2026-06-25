#include "Main.h"
#include "GameState_Win.h"
#include "GameState_Pong.h"
#include "AEParticle.h"
#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
extern "C" {
    EMSCRIPTEN_KEEPALIVE void DoRestart() { gGameStateNext = GS_PONG; }
}
#endif

static PongWinner sWinner = PONG_WINNER_NONE;

void GameState_WinLoad()
{
    // Pong was already unloaded by the state machine before this runs.
    // Re-load it so WinDraw can call PongDraw on the frozen board.
    GameState_PongLoad();
}

void GameState_WinInit()
{
    sWinner = PongGetWinner();
    // Deliberately NOT calling PongInit — we keep the frozen paddle/ball positions.
#if defined(__EMSCRIPTEN__)
    EM_ASM(
        var btn = document.getElementById('btn-restart');
        if (btn) btn.style.display = 'block';
    );
#endif
}

void GameState_WinUpdate()
{
    AEParticleUpdate((float)g_dt);
    if (AEInputCheckTriggered(AEVK_R))
        gGameStateNext = GS_PONG;  // full Load+Init cycle; resets scores, ball, particles
}

void GameState_WinDraw()
{
    GameState_PongDraw();  // frozen board: logo + particles + paddles + ball + scores

    // Win text overlay (BM_BLEND required for AEGfxPrint)
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    // Each string has a different length so each gets its own x to center it.
    if (sWinner == PONG_WINNER_PLAYER)
        AEGfxPrint(gFontId, "Elly's Awesomeness '_'",    -0.48f, -0.72f, 2.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    else
        AEGfxPrint(gFontId, "Elly's defeated by Bsen ;)", -0.58f, -0.72f, 2.0f, 1.0f, 1.0f, 0.0f, 1.0f);
    AEGfxPrint(gFontId, "Press R to restart",            -0.22f, -0.88f, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
}

void GameState_WinFree()
{
#if defined(__EMSCRIPTEN__)
    EM_ASM(
        var btn = document.getElementById('btn-restart');
        if (btn) btn.style.display = 'none';
    );
#endif
}

void GameState_WinUnload()
{
    GameState_PongUnload();
}
