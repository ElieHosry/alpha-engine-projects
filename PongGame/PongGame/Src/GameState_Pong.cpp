#include "Main.h"

static const float PADDLE_HALF_W  =  8.0f;
static const float PADDLE_HALF_H  = 50.0f;
static const float BALL_HALF_SIZE =  8.0f;
static const float PADDLE_INSET   = 360.0f;
static const float BALL_SPEED     = 250.0f;
static const float PLAYER_SPEED   = 300.0f;  // left paddle (player)
static const float AI_SPEED       = 220.0f;  // right paddle (auto-play, slightly slower so it's beatable)

static AEGfxVertexList* sMesh    = nullptr;
static AEGfxTexture*    sLogoTex = nullptr;

static Paddle sPaddleL;
static Paddle sPaddleR;
static Ball   sBall;

// Logo aspect ratio: 272 x 75 → drawn full window-width, proportional height
static const float LOGO_HALF_W = 400.0f;
static const float LOGO_HALF_H = 400.0f * (75.0f / 272.0f);  // ≈ 110

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
    // Slight upward incline (~20 degrees) heading right
    sBall.vel = { BALL_SPEED * 0.94f, BALL_SPEED * 0.34f };
}

// ----------------------------------------------------------------------------

void GameState_PongLoad()
{
    sMesh    = CreateUnitQuad();
    sLogoTex = AEGfxTextureLoad("../Resources/Textures/DigiPen_Logo.png");
}

void GameState_PongInit()
{
    AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

    sPaddleL = { { -PADDLE_INSET, 0.0f }, PADDLE_HALF_W, PADDLE_HALF_H };
    sPaddleR = { {  PADDLE_INSET, 0.0f }, PADDLE_HALF_W, PADDLE_HALF_H };

    sBall.halfSize = BALL_HALF_SIZE;
    ResetBall();
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
    if (AEInputCheckCurr(AEVK_W))
        sPaddleL.pos.y += PLAYER_SPEED * dt;
    if (AEInputCheckCurr(AEVK_S))
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
    // Ball movement
    // -------------------------------------------------------------------------
    sBall.pos.x += sBall.vel.x * dt;
    sBall.pos.y += sBall.vel.y * dt;

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

    if (sBall.vel.x < 0.0f && AABBOverlap(ballBox, leftBox))
    {
        sBall.pos.x = leftBox.maxX + sBall.halfSize;
        sBall.vel.x = -sBall.vel.x;
    }

    if (sBall.vel.x > 0.0f && AABBOverlap(ballBox, rightBox))
    {
        sBall.pos.x = rightBox.minX - sBall.halfSize;
        sBall.vel.x = -sBall.vel.x;
    }

    // Reset if ball exits left or right
    if (sBall.pos.x < -winMaxX || sBall.pos.x > winMaxX)
        ResetBall();
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

    // --- Game objects (opaque, color-only) ---
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxTextureSet(NULL, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    DrawRect(sPaddleL.pos.x, sPaddleL.pos.y, sPaddleL.halfW, sPaddleL.halfH);
    DrawRect(sPaddleR.pos.x, sPaddleR.pos.y, sPaddleR.halfW, sPaddleR.halfH);
    DrawRect(sBall.pos.x,    sBall.pos.y,    sBall.halfSize,  sBall.halfSize);
}

void GameState_PongFree()
{
}

void GameState_PongUnload()
{
    if (sLogoTex) { AEGfxTextureUnload(sLogoTex); sLogoTex = nullptr; }
    AEGfxMeshFree(sMesh);
    sMesh = nullptr;
}
