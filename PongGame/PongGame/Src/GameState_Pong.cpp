#include "Main.h"
#include "AEParticle.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>

static const float PADDLE_HALF_W  =  8.0f;
static const float PADDLE_HALF_H  = 50.0f;
static const float BALL_HALF_SIZE =  8.0f;
static const float PADDLE_INSET   = 576.0f;
static const float BALL_SPEED     = 250.0f;
static const float PLAYER_SPEED   = 290.0f;  // left paddle (player)
static const float AI_SPEED       = 400.0f;

static AEGfxVertexList* sMesh    = nullptr;
static AEGfxTexture*    sLogoTex = nullptr;

static Paddle sPaddleL;
static Paddle sPaddleR;
static Ball   sBall;

static int        sScorePlayer = 0;
static int        sScoreNPC    = 0;
static int        sBounceCount = 0;
static PongWinner sPongWinner  = PONG_WINNER_NONE;

// Logo aspect ratio: 272 x 75 → drawn full window-width, proportional height
static const float LOGO_HALF_W = 640.0f;
static const float LOGO_HALF_H = 640.0f * (75.0f / 272.0f);  // ≈ 176

static AEGfxVertexList* CreateUnitQuad()
{
    AEGfxMeshStart();
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
         0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(
        -0.5f,  0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
         0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
         0.5f,  0.5f, 0xFFFFFFFF, 1.0f, 0.0f);
    return AEGfxMeshEnd();
}

static void DrawRect(float cx, float cy, float hw, float hh)
{
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, hw * 2.0f, hh * 2.0f);
    AEMtx33Trans(&trans, cx, cy);
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(sMesh, AE_GFX_MDM_TRIANGLES);
}

static void ResetBall()
{
    sBall.pos = { 0.0f, 0.0f };
    // Base angle in [30°, 60°] avoids near-horizontal (0°/180°) and near-vertical (90°/270°).
    // Randomly mirror into one of the 4 quadrants for full 360° coverage.
    float baseAngle = 30.0f + (float)(rand() % 31);
    float rad = baseAngle * (3.14159265f / 180.0f);
    float vx  = cosf(rad);
    float vy  = sinf(rad);
    if (rand() % 2) vx = -vx;
    if (rand() % 2) vy = -vy;
    float speed = BALL_SPEED + (float)(rand() % (int)BALL_SPEED); // [BALL_SPEED, 2*BALL_SPEED)
    sBall.vel   = { speed * vx, speed * vy };
}

// ----------------------------------------------------------------------------

void GameState_PongLoad()
{
    sMesh    = CreateUnitQuad();
    sLogoTex = AEGfxTextureLoad("../Resources/Textures/DigiPen_Logo.png");
    AEParticleInit(256);
}

void GameState_PongInit()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

    sPaddleL = { { -PADDLE_INSET, 0.0f }, PADDLE_HALF_W, PADDLE_HALF_H };
    sPaddleR = { {  PADDLE_INSET, 0.0f }, PADDLE_HALF_W, PADDLE_HALF_H };

    sBall.halfSize = BALL_HALF_SIZE;
    ResetBall();

    sScorePlayer = 0;
    sScoreNPC    = 0;
    sBounceCount = 0;
    sPongWinner  = PONG_WINNER_NONE;
    AEParticleClear();
}

void GameState_PongUpdate()
{
    if (AEInputCheckTriggered(AEVK_ESCAPE))
    {
        gGameStateNext = GS_QUIT;
        return;
    }

    const float dt      = (float)g_dt;
    const float winMaxX = AEGfxGetWinMaxX();
    const float winMaxY = AEGfxGetWinMaxY();

    // -------------------------------------------------------------------------
    // Left paddle — player input (W = up, S = down)
    // -------------------------------------------------------------------------
    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP))
        sPaddleL.pos.y += PLAYER_SPEED * dt;
    if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN))
        sPaddleL.pos.y -= PLAYER_SPEED * dt;

    // Clamp to screen
    float paddleMaxY = winMaxY - sPaddleL.halfH;
    sPaddleL.pos.y = AEClamp(sPaddleL.pos.y, -paddleMaxY, paddleMaxY);

    // -------------------------------------------------------------------------
    // Right paddle — AI follows ball at capped speed
    // -------------------------------------------------------------------------
    float diff = sBall.pos.y - sPaddleR.pos.y;
    float move = AEClamp(diff, -AI_SPEED * dt, AI_SPEED * dt);
    sPaddleR.pos.y += move;
    sPaddleR.pos.y = AEClamp(sPaddleR.pos.y, -paddleMaxY, paddleMaxY);

    // -------------------------------------------------------------------------
    // Ball movement + particle trail
    // -------------------------------------------------------------------------
    sBall.pos.x += sBall.vel.x * dt;
    sBall.pos.y += sBall.vel.y * dt;

    AEParticleUpdate(dt);
    AEParticleEmit(sBall.pos.x, sBall.pos.y,
                   -20.0f, 20.0f, -20.0f, 20.0f,
                   0.0f, 0.85f, 1.0f,
                   3.0f, 1.0f, 3);

    // Top / bottom wall bounce
    if (sBall.pos.y + sBall.halfSize > winMaxY)
    {
        sBall.pos.y = winMaxY - sBall.halfSize;
        sBall.vel.y = -sBall.vel.y;
    }
    else if (sBall.pos.y - sBall.halfSize < -winMaxY)
    {
        sBall.pos.y = -winMaxY + sBall.halfSize;
        sBall.vel.y = -sBall.vel.y;
    }

    // Paddle AABB collision
    AABB ballBox  = sBall.GetAABB();
    AABB leftBox  = sPaddleL.GetAABB();
    AABB rightBox = sPaddleR.GetAABB();

    bool hitPaddle = false;
    if (sBall.vel.x < 0.0f && AABBOverlap(ballBox, leftBox))
    {
        sBall.pos.x = leftBox.maxX + sBall.halfSize;
        sBall.vel.x = -sBall.vel.x;
        hitPaddle   = true;
    }
    if (sBall.vel.x > 0.0f && AABBOverlap(ballBox, rightBox))
    {
        sBall.pos.x = rightBox.minX - sBall.halfSize;
        sBall.vel.x = -sBall.vel.x;
        hitPaddle   = true;
    }
    if (hitPaddle)
    {
        ++sBounceCount;
        if (sBounceCount % 4 == 0)
        {
            sBall.vel.x *= 1.15f;
            sBall.vel.y *= 1.15f;
            // Cap speed at 1600 units/s while preserving direction
            float spd = sqrtf(sBall.vel.x * sBall.vel.x + sBall.vel.y * sBall.vel.y);
            if (spd > 1600.0f)
            {
                float scale = 1600.0f / spd;
                sBall.vel.x *= scale;
                sBall.vel.y *= scale;
            }
        }
    }

    // Ball exits — score and reset (or trigger win)
    if (sBall.pos.x > winMaxX)
    {
        ++sScorePlayer;
        if (sScorePlayer >= 3) { sPongWinner = PONG_WINNER_PLAYER; gGameStateNext = GS_WIN; return; }
        AEParticleClear();
        ResetBall();
    }
    else if (sBall.pos.x < -winMaxX)
    {
        ++sScoreNPC;
        if (sScoreNPC >= 3) { sPongWinner = PONG_WINNER_NPC; gGameStateNext = GS_WIN; return; }
        AEParticleClear();
        ResetBall();
    }
}

void GameState_PongDraw()
{
    // --- DigiPen logo watermark (centered, full-width, semi-transparent) ---
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxTextureSet(sLogoTex, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(0.7f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    DrawRect(0.0f, 0.0f, LOGO_HALF_W, LOGO_HALF_H);

    // --- Particle trail (drawn before ball so ball renders on top) ---
    AEParticleDraw();

    // --- Game objects (opaque, per-object color) ---
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxTextureSet(NULL, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    // Player paddle — white
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    DrawRect(sPaddleL.pos.x, sPaddleL.pos.y, sPaddleL.halfW, sPaddleL.halfH);

    // NPC paddle — red
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    DrawRect(sPaddleR.pos.x, sPaddleR.pos.y, sPaddleR.halfW, sPaddleR.halfH);

    // Ball — blue
    AEGfxSetColorToMultiply(0.0f, 0.3f, 1.0f, 1.0f);
    DrawRect(sBall.pos.x, sBall.pos.y, sBall.halfSize, sBall.halfSize);

    // Scores + speed (AEGfxPrint requires BM_BLEND)
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    char buf[16];
    snprintf(buf, sizeof(buf), "P: %d", sScorePlayer);
    AEGfxPrint(gFontId, buf, -0.75f, 0.88f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    snprintf(buf, sizeof(buf), "NPC: %d", sScoreNPC);
    AEGfxPrint(gFontId, buf,  0.35f, 0.88f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
    float spd = sqrtf(sBall.vel.x * sBall.vel.x + sBall.vel.y * sBall.vel.y);
    snprintf(buf, sizeof(buf), "SPD:%d", (int)spd);
    AEGfxPrint(gFontId, buf, -0.12f, 0.88f, 1.0f, 0.0f, 0.3f, 1.0f, 1.0f);
}

void GameState_PongFree()
{
}

void GameState_PongUnload()
{
    AEParticleFree();
    if (sLogoTex) { AEGfxTextureUnload(sLogoTex); sLogoTex = nullptr; }
    AEGfxMeshFree(sMesh);
    sMesh = nullptr;
}

PongWinner PongGetWinner() { return sPongWinner; }
