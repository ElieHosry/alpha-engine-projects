# Pong Gameplay Modifications Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add per-object colors, ball speed escalation, a scoring system, a GS_WIN game state with frozen win screen, cyan particle trail, and a mobile RESTART button to Pong.

**Architecture:** A new reusable `AEParticle` module is added to `alpha_engine/` and auto-discovered by the web build's `file(GLOB)`. A new `GS_WIN` game state is registered in `GameStateMgr.cpp`; its Load/Unload delegate to Pong's Load/Unload so the frozen board can be re-rendered. All scoring and win-trigger logic lives in `GameState_Pong.cpp`.

**Tech Stack:** C++17, OpenGL/WebGL via Alpha Engine (`AEGfx*` API), SDL2, Emscripten (web target), Visual Studio 2022 (native target).

## Global Constraints

- One-source-two-targets rule: every `#if defined(__EMSCRIPTEN__)` divergence must be symmetric; no new divergence is introduced by these changes.
- `AEParticle.cpp` placed in `alpha_engine/src/` is auto-discovered by the web CMakeLists (`file(GLOB)`); it must also be manually added to `alpha_engine/build/mvs2022/alpha_engine.vcxproj`.
- `GameState_Win.cpp` must be added to `PongGame/PongGame/PongGame.vcxproj`.
- `AEGfxPrint` **requires** `AEGfxSetBlendMode(AE_GFX_BM_BLEND)` to be active before any call — set it explicitly.
- `AEGfxPrint` coordinates are normalized screen space: x and y both in range `[-1.0f, 1.0f]`.
- `rand()` is used for particle velocity; no seed call needed.
- Winning score threshold: 3 points.
- Ball speed multiplier per 4 bounces: ×1.15 (applied to both `vel.x` and `vel.y`).
- Particle color: `(0.0f, 0.85f, 1.0f)` — cyan. Particle lifetime: 1.0 s. Half-size: 3.0 f. Count per step: 3.

---

### Task 1: AEParticle Module

**Files:**
- Create: `AE/alpha_engine/include/AEParticle.h`
- Create: `AE/alpha_engine/src/AEParticle.cpp`
- Modify: `AE/alpha_engine/build/mvs2022/alpha_engine.vcxproj` (add .cpp and .h entries)

**Interfaces:**
- Produces:
  - `void AEParticleInit(int maxParticles)`
  - `void AEParticleFree()`
  - `void AEParticleUpdate(float dt)`
  - `void AEParticleDraw()`
  - `void AEParticleClear()`
  - `void AEParticleEmit(float x, float y, float vxMin, float vxMax, float vyMin, float vyMax, float r, float g, float b, float halfSize, float maxLife, int count)`

- [ ] **Step 1: Create `AEParticle.h`**

Write to `AE/alpha_engine/include/AEParticle.h`:

```cpp
#pragma once

void AEParticleInit(int maxParticles);
void AEParticleFree();
void AEParticleUpdate(float dt);
void AEParticleDraw();
void AEParticleClear();
void AEParticleEmit(float x, float y,
                    float vxMin, float vxMax,
                    float vyMin, float vyMax,
                    float r, float g, float b,
                    float halfSize, float maxLife, int count);
```

- [ ] **Step 2: Create `AEParticle.cpp`**

Write to `AE/alpha_engine/src/AEParticle.cpp`:

```cpp
#include "AEParticle.h"
#include "AEEngine.h"
#include <cstdlib>

struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float halfSize;
    float life;
    float maxLife;
    bool  alive;
};

static Particle*        sPool = nullptr;
static int              sMax  = 0;
static AEGfxVertexList* sMesh = nullptr;

static float randRange(float lo, float hi)
{
    return lo + (hi - lo) * (rand() / (float)RAND_MAX);
}

static AEGfxVertexList* CreateParticleMesh()
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

void AEParticleInit(int maxParticles)
{
    sMax  = maxParticles;
    sPool = new Particle[sMax];
    AEParticleClear();
    sMesh = CreateParticleMesh();
}

void AEParticleFree()
{
    if (sMesh) { AEGfxMeshFree(sMesh); sMesh = nullptr; }
    delete[] sPool;
    sPool = nullptr;
    sMax  = 0;
}

void AEParticleClear()
{
    for (int i = 0; i < sMax; ++i)
        sPool[i].alive = false;
}

void AEParticleUpdate(float dt)
{
    for (int i = 0; i < sMax; ++i)
    {
        if (!sPool[i].alive) continue;
        sPool[i].life -= dt;
        if (sPool[i].life <= 0.0f) { sPool[i].alive = false; continue; }
        sPool[i].x += sPool[i].vx * dt;
        sPool[i].y += sPool[i].vy * dt;
    }
}

void AEParticleDraw()
{
    if (!sPool || !sMesh) return;

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxTextureSet(NULL, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    for (int i = 0; i < sMax; ++i)
    {
        if (!sPool[i].alive) continue;
        float alpha = sPool[i].life / sPool[i].maxLife;
        AEGfxSetColorToMultiply(sPool[i].r, sPool[i].g, sPool[i].b, 1.0f);
        AEGfxSetTransparency(alpha);

        AEMtx33 scale, trans, xform;
        AEMtx33Scale(&scale, sPool[i].halfSize * 2.0f, sPool[i].halfSize * 2.0f);
        AEMtx33Trans(&trans, sPool[i].x, sPool[i].y);
        AEMtx33Concat(&xform, &trans, &scale);
        AEGfxSetTransform(xform.m);
        AEGfxMeshDraw(sMesh, AE_GFX_MDM_TRIANGLES);
    }
}

void AEParticleEmit(float x, float y,
                    float vxMin, float vxMax,
                    float vyMin, float vyMax,
                    float r, float g, float b,
                    float halfSize, float maxLife, int count)
{
    int spawned = 0;
    for (int i = 0; i < sMax && spawned < count; ++i)
    {
        if (sPool[i].alive) continue;
        sPool[i].x        = x;
        sPool[i].y        = y;
        sPool[i].vx       = randRange(vxMin, vxMax);
        sPool[i].vy       = randRange(vyMin, vyMax);
        sPool[i].r        = r;
        sPool[i].g        = g;
        sPool[i].b        = b;
        sPool[i].halfSize = halfSize;
        sPool[i].maxLife  = maxLife;
        sPool[i].life     = maxLife;
        sPool[i].alive    = true;
        ++spawned;
    }
}
```

- [ ] **Step 3: Add AEParticle to the VS engine project**

Open `AE/alpha_engine/build/mvs2022/alpha_engine.vcxproj` in a text editor.

Find the line: `<ClCompile Include="..\..\src\AEInput.cpp" />`
After it, add:
```xml
    <ClCompile Include="..\..\src\AEParticle.cpp" />
```

Find the line: `<ClInclude Include="..\..\include\AEGraphics.h" />`
After it (or near other `ClInclude` entries), add:
```xml
    <ClInclude Include="..\..\include\AEParticle.h" />
```

- [ ] **Step 4: Verify — native build compiles**

Open `alpha_engine.vcxproj` in Visual Studio 2022. Build (any config). Expected: 0 errors. The AEParticle translation unit should appear in the build output.

---

### Task 2: GS_WIN State Registration

**Files:**
- Modify: `AE/PongGame/PongGame/Include/GameStateList.h`
- Create: `AE/PongGame/PongGame/Include/GameState_Win.h`
- Create: `AE/PongGame/PongGame/Src/GameState_Win.cpp` (stub — fully implemented in Task 4)
- Modify: `AE/PongGame/PongGame/Src/GameStateMgr.cpp`
- Modify: `AE/PongGame/PongGame/PongGame.vcxproj`

**Interfaces:**
- Consumes: nothing yet (stubs)
- Produces:
  - `GS_WIN` constant (= 3)
  - `GameState_WinLoad / Init / Update / Draw / Free / Unload` (stubs, wired into state machine)

- [ ] **Step 1: Add GS_WIN to GameStateList.h**

Replace the full content of `AE/PongGame/PongGame/Include/GameStateList.h` with:

```cpp
#ifndef PONG_GAMESTATELIST_H_
#define PONG_GAMESTATELIST_H_

enum
{
    GS_PONG = 0,

    // Reserved — do not change order
    GS_RESTART,
    GS_QUIT,

    GS_WIN
};

#endif // PONG_GAMESTATELIST_H_
```

- [ ] **Step 2: Create GameState_Win.h**

Write to `AE/PongGame/PongGame/Include/GameState_Win.h`:

```cpp
#ifndef PONG_GAMESTATE_WIN_H_
#define PONG_GAMESTATE_WIN_H_

void GameState_WinLoad();
void GameState_WinInit();
void GameState_WinUpdate();
void GameState_WinDraw();
void GameState_WinFree();
void GameState_WinUnload();

#endif // PONG_GAMESTATE_WIN_H_
```

- [ ] **Step 3: Create GameState_Win.cpp (stubs)**

Write to `AE/PongGame/PongGame/Src/GameState_Win.cpp`:

```cpp
#include "Main.h"
#include "GameState_Win.h"

void GameState_WinLoad()   {}
void GameState_WinInit()   {}
void GameState_WinUpdate() {}
void GameState_WinDraw()   {}
void GameState_WinFree()   {}
void GameState_WinUnload() {}
```

- [ ] **Step 4: Wire GS_WIN in GameStateMgr.cpp**

Replace the full content of `AE/PongGame/PongGame/Src/GameStateMgr.cpp` with:

```cpp
#include "GameStateMgr.h"
#include "GameState_Pong.h"
#include "GameState_Win.h"

unsigned int gGameStateInit = 0;
unsigned int gGameStateCurr = 0;
unsigned int gGameStateNext = 0;
unsigned int gGameStatePrev = 0;

void (*GameStateLoad)()   = nullptr;
void (*GameStateInit)()   = nullptr;
void (*GameStateUpdate)() = nullptr;
void (*GameStateDraw)()   = nullptr;
void (*GameStateFree)()   = nullptr;
void (*GameStateUnload)() = nullptr;

void GameStateMgrInit(unsigned int gameStateInit)
{
    gGameStateCurr = gGameStateNext = gGameStatePrev = gGameStateInit = gameStateInit;
}

void GameStateMgrUpdate()
{
    switch (gGameStateCurr)
    {
    case GS_PONG:
        GameStateLoad   = GameState_PongLoad;
        GameStateInit   = GameState_PongInit;
        GameStateUpdate = GameState_PongUpdate;
        GameStateDraw   = GameState_PongDraw;
        GameStateFree   = GameState_PongFree;
        GameStateUnload = GameState_PongUnload;
        break;

    case GS_WIN:
        GameStateLoad   = GameState_WinLoad;
        GameStateInit   = GameState_WinInit;
        GameStateUpdate = GameState_WinUpdate;
        GameStateDraw   = GameState_WinDraw;
        GameStateFree   = GameState_WinFree;
        GameStateUnload = GameState_WinUnload;
        break;

    default:
        AE_ASSERT_MESG(false, "Invalid game state.");
        break;
    }
}
```

- [ ] **Step 5: Add GameState_Win to PongGame.vcxproj**

Open `AE/PongGame/PongGame/PongGame.vcxproj` in a text editor.

Find: `<ClCompile Include="Src\GameStateMgr.cpp" />`
After it, add:
```xml
    <ClCompile Include="Src\GameState_Win.cpp" />
```

Find: `<ClInclude Include="Include\GameState_Pong.h" />`
After it, add:
```xml
    <ClInclude Include="Include\GameState_Win.h" />
```

- [ ] **Step 6: Verify — native build compiles**

Build the PongGame VS project (Release x64). Expected: 0 errors. Run the .exe — game behaves identically to before (no GS_WIN is ever triggered yet).

---

### Task 3: Pong Colors, Scoring, Speed Escalation, Win Trigger

**Files:**
- Modify: `AE/PongGame/PongGame/Include/GameState_Pong.h`
- Modify: `AE/PongGame/PongGame/Src/GameState_Pong.cpp`

**Interfaces:**
- Consumes: `GS_WIN` from GameStateList.h (via Main.h)
- Produces:
  - `enum PongWinner { PONG_WINNER_NONE=0, PONG_WINNER_PLAYER, PONG_WINNER_NPC }`
  - `PongWinner PongGetWinner()` — returns winner at time of win; used by GameState_Win in Task 4

- [ ] **Step 1: Add PongWinner enum and getter declaration to GameState_Pong.h**

In `AE/PongGame/PongGame/Include/GameState_Pong.h`, add before the function declarations (before `void GameState_PongLoad();`):

```cpp
enum PongWinner { PONG_WINNER_NONE = 0, PONG_WINNER_PLAYER, PONG_WINNER_NPC };
PongWinner PongGetWinner();
```

The top of the file already has `#include "AEEngine.h"` — leave everything else intact.

- [ ] **Step 2: Add new statics and #include to GameState_Pong.cpp**

At the top of `AE/PongGame/PongGame/Src/GameState_Pong.cpp`, after `#include "Main.h"`, add:

```cpp
#include <cstdio>
```

After the existing static declarations (`static Ball sBall;` block), add:

```cpp
static int        sScorePlayer = 0;
static int        sScoreNPC    = 0;
static int        sBounceCount = 0;
static PongWinner sPongWinner  = PONG_WINNER_NONE;
```

- [ ] **Step 3: Reset new statics in GameState_PongInit**

In `GameState_PongInit()`, after `ResetBall();`, add:

```cpp
sScorePlayer = 0;
sScoreNPC    = 0;
sBounceCount = 0;
sPongWinner  = PONG_WINNER_NONE;
```

- [ ] **Step 4: Add bounce counter and speed escalation in GameState_PongUpdate**

In `GameState_PongUpdate()`, find the two paddle collision blocks:

```cpp
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
```

Replace them with:

```cpp
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
    }
}
```

- [ ] **Step 5: Add scoring and win trigger in GameState_PongUpdate**

Find the ball-exit block:

```cpp
// Reset if ball exits left or right
if (sBall.pos.x < -winMaxX || sBall.pos.x > winMaxX)
    ResetBall();
```

Replace it with:

```cpp
if (sBall.pos.x > winMaxX)
{
    ++sScorePlayer;
    if (sScorePlayer >= 3) { sPongWinner = PONG_WINNER_PLAYER; gGameStateNext = GS_WIN; return; }
    ResetBall();
}
else if (sBall.pos.x < -winMaxX)
{
    ++sScoreNPC;
    if (sScoreNPC >= 3) { sPongWinner = PONG_WINNER_NPC; gGameStateNext = GS_WIN; return; }
    ResetBall();
}
```

- [ ] **Step 6: Apply per-object colors and score display in GameState_PongDraw**

Find the "Game objects" draw block:

```cpp
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
```

Replace it with:

```cpp
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

// Scores (AEGfxPrint requires BM_BLEND)
AEGfxSetBlendMode(AE_GFX_BM_BLEND);
char buf[16];
sprintf(buf, "P: %d", sScorePlayer);
AEGfxPrint(gFontId, buf, -0.75f, 0.88f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
sprintf(buf, "NPC: %d", sScoreNPC);
AEGfxPrint(gFontId, buf,  0.35f, 0.88f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
```

- [ ] **Step 7: Add PongGetWinner at the bottom of GameState_Pong.cpp**

After `GameState_PongUnload()`, add:

```cpp
PongWinner PongGetWinner() { return sPongWinner; }
```

- [ ] **Step 8: Verify — build and run**

Build Release x64, run `PongGame.exe`.

Check:
- Left paddle is white, right paddle is red, ball is blue.
- "P: 0" appears top-left in white; "NPC: 0" appears top-right in red.
- Score increments when ball exits the respective side.
- After 4 paddle bounces the ball visibly accelerates.
- At score 3, transition occurs (blank/stub win screen — that's expected at this stage, Win is not yet implemented).

---

### Task 4: Win Screen Implementation

**Files:**
- Modify: `AE/PongGame/PongGame/Src/GameState_Win.cpp` (replace stubs with full implementation)

**Interfaces:**
- Consumes:
  - `PongGetWinner()` → `PongWinner` (from `GameState_Pong.h` via `Main.h`)
  - `GameState_PongLoad()`, `GameState_PongDraw()`, `GameState_PongUnload()` (via `Main.h`)
  - `AEParticleUpdate(float dt)` (from `AEParticle.h`)
  - `gFontId` (extern `s8` in `Main.h`)
  - `g_dt` (extern `f64` in `Main.h`)
  - `gGameStateNext`, `GS_PONG` (via `Main.h` → `GameStateMgr.h` + `GameStateList.h`)
  - `AEVK_ESCAPE`, `AEInputCheckTriggered` (via `AEEngine.h`)

**State transition note:** When `GS_PONG → GS_WIN`, the engine calls `PongFree` (no-op) then `PongUnload` (frees mesh/texture/particles), then `WinLoad`. `WinLoad` calls `PongLoad` to recreate them so `WinDraw` can call `PongDraw` to render the frozen board. `WinInit` does NOT call `PongInit` — frozen positions are preserved. On restart (`GS_WIN → GS_PONG`), `WinUnload` calls `PongUnload`, then the engine calls `PongLoad + PongInit`, fully resetting everything.

- [ ] **Step 1: Replace GameState_Win.cpp with full implementation**

Overwrite `AE/PongGame/PongGame/Src/GameState_Win.cpp` with:

```cpp
#include "Main.h"
#include "GameState_Win.h"
#include "GameState_Pong.h"
#include "AEParticle.h"

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
}

void GameState_WinUpdate()
{
    AEParticleUpdate((float)g_dt);
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        gGameStateNext = GS_PONG;  // full Load+Init cycle; resets scores, ball, particles
}

void GameState_WinDraw()
{
    GameState_PongDraw();  // frozen board: logo + particles + paddles + ball + scores

    // Win text overlay (BM_BLEND required for AEGfxPrint)
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    const char* line1 = (sWinner == PONG_WINNER_PLAYER)
        ? "Elly's Awesomeness '_'"
        : "Elly's defeated by Bsen ;)";
    AEGfxPrint(gFontId, line1,                -0.55f,  0.15f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxPrint(gFontId, "Press ESC to restart", -0.35f, -0.10f, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
}

void GameState_WinFree()   {}

void GameState_WinUnload()
{
    GameState_PongUnload();
}
```

- [ ] **Step 2: Verify — build and run**

Build Release x64, run the .exe. Play until either side reaches 3 points.

Check:
- Win screen shows the frozen board (paddles, ball, scores at final values).
- Correct text appears: `"Elly's Awesomeness '_'"` if player wins, `"Elly's defeated by Bsen ;)"` if NPC wins.
- "Press ESC to restart" hint appears below.
- Pressing Escape restarts the game from 0-0 with default ball speed.
- If text is misaligned, tweak the x/y values in `WinDraw` (recall: x/y are normalized, range -1 to 1).

---

### Task 5: Integrate AEParticle into Pong

**Files:**
- Modify: `AE/PongGame/PongGame/Src/GameState_Pong.cpp`

**Interfaces:**
- Consumes: all six `AEParticle*` functions from `AEParticle.h`

- [ ] **Step 1: Add AEParticle.h include**

In `AE/PongGame/PongGame/Src/GameState_Pong.cpp`, after `#include <cstdio>`, add:

```cpp
#include "AEParticle.h"
```

- [ ] **Step 2: Init the particle pool in GameState_PongLoad**

In `GameState_PongLoad()`, after the `sLogoTex = ...` line, add:

```cpp
AEParticleInit(256);
```

Full function:
```cpp
void GameState_PongLoad()
{
    sMesh    = CreateUnitQuad();
    sLogoTex = AEGfxTextureLoad("../Resources/Textures/DigiPen_Logo.png");
    AEParticleInit(256);
}
```

- [ ] **Step 3: Clear particles on PongInit (reset)**

In `GameState_PongInit()`, after `sPongWinner = PONG_WINNER_NONE;`, add:

```cpp
AEParticleClear();
```

- [ ] **Step 4: Update and emit particles in GameState_PongUpdate**

In `GameState_PongUpdate()`, after the ball movement lines (`sBall.pos.x += ...` and `sBall.pos.y += ...`), add:

```cpp
AEParticleUpdate(dt);
AEParticleEmit(sBall.pos.x, sBall.pos.y,
               -20.0f, 20.0f, -20.0f, 20.0f,
               0.0f, 0.85f, 1.0f,
               3.0f, 1.0f, 3);
```

Also add `AEParticleClear()` before `ResetBall()` in both scoring branches:

Find:
```cpp
if (sBall.pos.x > winMaxX)
{
    ++sScorePlayer;
    if (sScorePlayer >= 3) { sPongWinner = PONG_WINNER_PLAYER; gGameStateNext = GS_WIN; return; }
    ResetBall();
}
else if (sBall.pos.x < -winMaxX)
{
    ++sScoreNPC;
    if (sScoreNPC >= 3) { sPongWinner = PONG_WINNER_NPC; gGameStateNext = GS_WIN; return; }
    ResetBall();
}
```

Replace with:
```cpp
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
```

- [ ] **Step 5: Draw particles in GameState_PongDraw**

In `GameState_PongDraw()`, after the logo draw block and before the game-objects block (before the `AEGfxSetRenderMode(AE_GFX_RM_COLOR)` line of game objects), add:

```cpp
// --- Particle trail (drawn before ball so ball renders on top) ---
AEParticleDraw();
```

- [ ] **Step 6: Free particles in GameState_PongUnload**

In `GameState_PongUnload()`, add `AEParticleFree()` before the existing cleanup:

```cpp
void GameState_PongUnload()
{
    AEParticleFree();
    if (sLogoTex) { AEGfxTextureUnload(sLogoTex); sLogoTex = nullptr; }
    AEGfxMeshFree(sMesh);
    sMesh = nullptr;
}
```

- [ ] **Step 7: Verify — build and run**

Build Release x64, run the .exe.

Check:
- A cyan trail of small fading squares follows the ball as it moves.
- Trail disappears when ball resets to center after a score.
- Win screen still works — pressing Escape from win clears all particles on restart.

---

### Task 6: Mobile RESTART Button (shell.html)

**Files:**
- Modify: `AE/PongGame/PongGame/web/shell.html`

The button appears on touch devices only (pointer:coarse), positioned at the bottom-center of the screen. It dispatches synthetic Escape keypresses, which the C++ handles as: quit during gameplay, restart during win screen.

- [ ] **Step 1: Add CSS for the RESTART button**

In `shell.html`, inside the `<style>` block, after the `progress { ... }` rule, add:

```css
    #btn-restart {
      position: fixed; bottom: 24px; left: 50%; transform: translateX(-50%);
      padding: 10px 32px; font-size: 18px; font-family: system-ui, sans-serif;
      background: #111; color: #fff; border: 2px solid #fff; border-radius: 6px;
      cursor: pointer; z-index: 10; touch-action: manipulation; display: none;
    }
    @media (pointer: fine) { #btn-restart { display: none !important; } }
```

- [ ] **Step 2: Add the button element to the DOM**

In `shell.html`, inside `<div class="wrap">`, after the `<canvas>` element, add:

```html
    <button id="btn-restart">RESTART</button>
```

- [ ] **Step 3: Show button on load and add Escape dispatch**

In `shell.html`, replace the existing `Module.setStatus` function:

```js
      setStatus: function (text) {
        statusEl.textContent = text;
        if (!text) progressEl.style.display = 'none';
      },
```

with:

```js
      setStatus: function (text) {
        statusEl.textContent = text;
        if (!text) {
          progressEl.style.display = 'none';
          // Show RESTART button only on touch (coarse pointer) devices
          if (window.matchMedia('(pointer: coarse)').matches)
            document.getElementById('btn-restart').style.display = 'block';
        }
      },
```

Then, inside the swipe-controls IIFE (the `(function () { ... })();` block), at the end just before the closing `})();`, add:

```js
      document.getElementById('btn-restart').addEventListener('pointerdown', function (e) {
        e.preventDefault();
        var esc = { key: 'Escape', code: 'Escape', keyCode: 27, which: 27, bubbles: true, cancelable: true };
        window.dispatchEvent(new KeyboardEvent('keydown', esc));
        window.dispatchEvent(new KeyboardEvent('keyup',   esc));
      });
```

- [ ] **Step 4: Build the web version**

From `AE/PongGame/PongGame/web/`, run:

```bat
build.bat Release
```

Expected: build succeeds, `out/Pong.html` + `Pong.js` + `Pong.wasm` + `Pong.data` are produced.

- [ ] **Step 5: Serve and test on mobile (or browser DevTools)**

```bat
emrun out\Pong.html
```

Open on a phone or open DevTools → toggle device toolbar (Cmd+Shift+M / Ctrl+Shift+M).

Check:
- RESTART button appears at bottom of screen after game loads.
- During gameplay, tapping RESTART closes/quits (Escape → GS_QUIT on web, which calls `emscripten_cancel_main_loop`).
- After reaching score 3, win screen appears, tapping RESTART restarts the game.
- The button is not visible on desktop (pointer:fine media query hides it).

- [ ] **Step 6: Deploy to GitHub Pages**

```bat
redeploy.bat "add gameplay mods: colors, scoring, particles, win screen"
```

Expected output ends with: `Deployed. GitHub Pages will update in ~1 minute.`

---

## Self-Review

**Spec coverage check:**

| Spec requirement | Task |
|---|---|
| Red NPC paddle | Task 3, Step 6 |
| Blue ball | Task 3, Step 6 |
| Ball speed ×1.15 every 4 bounces | Task 3, Steps 4 |
| White player score / red NPC score at top | Task 3, Step 6 |
| Win at score 3 → GS_WIN | Task 3, Step 5 |
| Win text "Elly's Awesomeness '_'" / "Elly's defeated by Bsen ;)" | Task 4, Step 1 |
| Freeze ball and paddles on win | Task 4, Step 1 (WinUpdate has no movement calls) |
| Escape → restart on PC | Task 4, Step 1 |
| RESTART button on mobile web | Task 6 |
| Cyan particle trail fading over 1 s | Tasks 1 + 5 |
| AEParticle reusable in alpha_engine | Task 1 |
| GS_WIN as separate game state | Task 2 |

All requirements covered. No gaps.
