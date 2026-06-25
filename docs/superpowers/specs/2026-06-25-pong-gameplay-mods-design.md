# Pong Gameplay Modifications Design

**Date:** 2026-06-25
**Project:** Alpha Engine — PongGame

---

## Overview

Add six gameplay features to Pong: per-object colors, ball speed escalation, a scoring system, a win screen with a separate game state, trailing particle effects for the ball, and a mobile-friendly RESTART button. A reusable `AEParticle` module is added to the Alpha Engine.

---

## Files Modified / Created

| File | Action | Purpose |
|---|---|---|
| `PongGame/Include/GameStateList.h` | Modify | Add `GS_WIN = 3` |
| `PongGame/Include/GameState_Pong.h` | Modify | Add `PongWinner` enum + getter declaration |
| `PongGame/Src/GameState_Pong.cpp` | Modify | Colors, bounce tracking, scoring, win trigger, particle emit |
| `PongGame/Include/GameStateMgr.h` | No change | — |
| `PongGame/Src/GameStateMgr.cpp` | Modify | Add `GS_WIN` case wiring Win state functions |
| `PongGame/Include/GameState_Win.h` | **Create** | Win state function declarations |
| `PongGame/Src/GameState_Win.cpp` | **Create** | Win screen: frozen draw + text overlay + restart |
| `AE/alpha_engine/include/AEParticle.h` | **Create** | Reusable particle system API |
| `AE/alpha_engine/src/AEParticle.cpp` | **Create** | Particle system implementation |
| `PongGame/web/shell.html` | Modify | RESTART button for mobile |

---

## 1. Particle System — `AEParticle.h/.cpp`

Lives in `alpha_engine/` so any future game can reuse it.

### API

```cpp
// AEParticle.h
#pragma once

// Call once at game load; allocates internal pool of up to maxParticles.
void AEParticleInit(int maxParticles);

// Call at game unload; frees mesh and resets pool.
void AEParticleFree();

// Advance lifetimes and positions. Call once per fixed timestep.
void AEParticleUpdate(float dt);

// Render all live particles. Uses AE_GFX_BM_BLEND + transparency fade.
void AEParticleDraw();

// Kill all live particles instantly (e.g., on game restart).
void AEParticleClear();

// Spawn `count` particles at (x,y) with random velocity in [vxMin,vxMax] x [vyMin,vyMax].
// Color (r,g,b) is fixed per batch; alpha fades from 1.0 to 0.0 over maxLife seconds.
void AEParticleEmit(float x, float y,
                    float vxMin, float vxMax,
                    float vyMin, float vyMax,
                    float r, float g, float b,
                    float halfSize, float maxLife, int count);
```

### Internal structure

```cpp
struct Particle {
    float x, y;
    float vx, vy;
    float r, g, b;
    float halfSize;
    float life;     // seconds remaining
    float maxLife;  // for alpha = life/maxLife
    bool  alive;
};
```

Static pool: `Particle sPool[sMaxParticles]` (no heap allocation beyond the initial `new[]` in `AEParticleInit`). `AEParticleEmit` linearly scans for dead slots; if the pool is full, new particles are silently dropped.

`AEParticleDraw` renders each live particle as a unit quad (the module owns its own `AEGfxVertexList*` mesh created in `AEParticleInit`):
- `AE_GFX_RM_COLOR`, `AE_GFX_BM_BLEND`
- `AEGfxSetColorToMultiply(r, g, b, 1.0f)`
- `AEGfxSetTransparency(life / maxLife)`
- One `AEGfxMeshDraw` call per live particle

Random velocity: `rand() % range + min` (integer-quantized to ±1 px/s buckets is fine for the visual effect).

### Pong usage

| Call | Where |
|---|---|
| `AEParticleInit(256)` | `GameState_PongLoad` |
| `AEParticleFree()` | `GameState_PongUnload` |
| `AEParticleClear()` | `GameState_PongInit` (reset on restart) |
| `AEParticleUpdate((float)g_dt)` | `GameState_PongUpdate`, each fixed step |
| `AEParticleEmit(ball.x, ball.y, -20,20, -20,20, 0.0f,0.85f,1.0f, 3.0f, 1.0f, 3)` | `GameState_PongUpdate`, after ball movement |
| `AEParticleDraw()` | `GameState_PongDraw`, after logo, before paddles/ball |
| `AEParticleUpdate((float)g_dt)` | `GameState_WinUpdate` (trail continues fading during win screen) |

Particle color: `(0.0, 0.85, 1.0)` — cyan, slightly brighter than the blue ball `(0.0, 0.3, 1.0)`.
Particle size: `halfSize = 3.0f` (slightly smaller than ball's `halfSize = 8.0f`).
Lifetime: `1.0f` seconds.

---

## 2. Object Colors

Applied via `AEGfxSetColorToMultiply` before each draw call in `GameState_PongDraw`. Vertex color is white (0xFFFFFFFF), so the multiply color maps directly to the rendered color.

| Object | Color | `AEGfxSetColorToMultiply` args |
|---|---|---|
| Player paddle (left) | White | `(1.0, 1.0, 1.0, 1.0)` |
| NPC paddle (right) | Red | `(1.0, 0.0, 0.0, 1.0)` |
| Ball | Blue | `(0.0, 0.3, 1.0, 1.0)` |

`AEGfxSetColorToAdd` stays `(0,0,0,0)` for all.

---

## 3. Scoring System

### State

Two new statics in `GameState_Pong.cpp`:
```cpp
static int sScorePlayer = 0;  // left paddle wins a rally
static int sScoreNPC    = 0;  // right paddle wins a rally
```

Both reset to 0 in `GameState_PongInit`.

### Scoring logic (in `GameState_PongUpdate`)

Replace the current `ResetBall()` call when the ball exits bounds with:

```
if ball exits RIGHT side (pos.x > winMaxX):
    sScorePlayer++
    if sScorePlayer >= 3: sPongWinner = PONG_WINNER_PLAYER; gGameStateNext = GS_WIN; return
    AEParticleClear(); ResetBall()

if ball exits LEFT side (pos.x < -winMaxX):
    sScoreNPC++
    if sScoreNPC >= 3: sPongWinner = PONG_WINNER_NPC; gGameStateNext = GS_WIN; return
    AEParticleClear(); ResetBall()
```

### Score display (in `GameState_PongDraw`)

Drawn after all geometry, at top of viewport:

```cpp
char buf[16];

// Player score — white, left side
sprintf(buf, "P: %d", sScorePlayer);
AEGfxPrint(gFontId, buf, -0.75f, 0.88f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

// NPC score — red, right side
sprintf(buf, "NPC: %d", sScoreNPC);
AEGfxPrint(gFontId, buf,  0.35f, 0.88f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);
```

---

## 4. Ball Speed Escalation

New static in `GameState_Pong.cpp`:
```cpp
static int sBounceCount = 0;
```

Reset to 0 in `GameState_PongInit`.

On each paddle collision (both left and right), after reversing `vel.x`:
```cpp
++sBounceCount;
if (sBounceCount % 4 == 0) {
    sBall.vel.x *= 1.15f;
    sBall.vel.y *= 1.15f;
}
```

Speed factor: ×1.15 every 4 bounces (~52% faster after 12 bounces). NPC AI speed stays at constant `AI_SPEED = 220`, so it naturally falls behind as the ball accelerates.

---

## 5. Win State (`GS_WIN`)

### GameStateList.h

```cpp
enum {
    GS_PONG = 0,
    GS_RESTART,
    GS_QUIT,
    GS_WIN       // new — win screen overlay
};
```

### GameState_Pong.h additions

```cpp
enum PongWinner { PONG_WINNER_NONE = 0, PONG_WINNER_PLAYER, PONG_WINNER_NPC };
PongWinner PongGetWinner();
```

`PongGetWinner()` returns the static `sPongWinner` variable set at win detection. Defined in `GameState_Pong.cpp`.

### GameState_Win.h

```cpp
#pragma once
void GameState_WinLoad();
void GameState_WinInit();
void GameState_WinUpdate();
void GameState_WinDraw();
void GameState_WinFree();
void GameState_WinUnload();
```

### GameState_Win.cpp

**Load/Unload:** no-op. Pong's mesh and textures remain loaded (Pong is not unloaded when transitioning to Win — the state machine calls `PongFree` then switches to Win without calling `PongUnload`).

> **Note on state transitions:** `GameStateMgr.cpp` calls `GameStateFree` then (if next ≠ GS_RESTART) `GameStateUnload`. When transitioning `GS_PONG → GS_WIN`, `PongFree` is called (no-op) and `PongUnload` IS called — this would free Pong's mesh. To avoid this, Win's `Load` function must recreate the particle mesh (or the particle system is self-contained and this is fine). Actually: `AEParticle` owns its own mesh and is freed in `PongUnload`. Win's draw only needs the frozen draw via `GameState_PongDraw` — but if Pong is unloaded, PongDraw's mesh is freed.

**Revised approach:** Win's `WinLoad` calls `GameState_PongLoad()` itself (re-creates mesh and particle system). `WinUnload` calls `GameState_PongUnload()`. This ensures the board is fully renderable. `WinInit` does NOT call `PongInit` (we want to keep the frozen positions and scores visible).

**Init:** reads `PongGetWinner()`, stores in a local static `sWinner`.

**Update:**
```cpp
void GameState_WinUpdate() {
    AEParticleUpdate((float)g_dt);  // trail fades out
    if (AEInputCheckTriggered(AEVK_ESCAPE))
        gGameStateNext = GS_PONG;   // full Load+Init cycle, resets everything
}
```

Using `GS_PONG` (not `GS_RESTART`) so that `PongLoad` and `PongInit` both run, fully resetting scores, bounce count, ball, and particles.

**Draw:**
```cpp
void GameState_WinDraw() {
    GameState_PongDraw();  // frozen board (logo + particles + paddles + ball + scores)

    // Win text — centered, large
    const char* line1 = (sWinner == PONG_WINNER_PLAYER)
        ? "Elly's Awesomeness '_'"
        : "Elly's defeated by Bsen ;)";
    AEGfxPrint(gFontId, line1, -0.55f, 0.15f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);

    // Restart hint — smaller, grey
    AEGfxPrint(gFontId, "Press ESC to restart", -0.35f, -0.1f, 1.0f, 0.6f, 0.6f, 0.6f, 1.0f);
}
```

x/y positions and scale for the win text will be tuned during implementation to center correctly for each string's length.

### GameStateMgr.cpp additions

```cpp
case GS_WIN:
    GameStateLoad   = GameState_WinLoad;
    GameStateInit   = GameState_WinInit;
    GameStateUpdate = GameState_WinUpdate;
    GameStateDraw   = GameState_WinDraw;
    GameStateFree   = GameState_WinFree;
    GameStateUnload = GameState_WinUnload;
    break;
```

---

## 6. RESTART Button (Mobile Web — `shell.html`)

A fixed HTML button, hidden on desktop (pointer:fine), visible on touch devices. Dispatches synthetic Escape key so it drives the same C++ input path.

```css
#btn-restart {
    position: fixed;
    bottom: 24px;
    left: 50%;
    transform: translateX(-50%);
    padding: 10px 32px;
    font-size: 18px;
    font-family: system-ui, sans-serif;
    background: #111;
    color: #fff;
    border: 2px solid #fff;
    border-radius: 6px;
    cursor: pointer;
    z-index: 10;
    touch-action: none;
    display: none; /* shown only on touch after WASM loads */
}
@media (pointer: fine) { #btn-restart { display: none !important; } }
```

Show the button once WASM runtime is ready (via `Module.onRuntimeInitialized` or after `setStatus('')` is called):

```js
Module.setStatus = function(text) {
    statusEl.textContent = text;
    if (!text) {
        progressEl.style.display = 'none';
        // Reveal RESTART on touch devices only
        if (window.matchMedia('(pointer: coarse)').matches)
            document.getElementById('btn-restart').style.display = 'block';
    }
};
```

Button handler:
```js
document.getElementById('btn-restart').addEventListener('pointerdown', function(e) {
    e.preventDefault();
    var esc = { key: 'Escape', code: 'Escape', keyCode: 27, which: 27, bubbles: true, cancelable: true };
    window.dispatchEvent(new KeyboardEvent('keydown', esc));
    window.dispatchEvent(new KeyboardEvent('keyup',   esc));
});
```

---

## Constraints

- All C++ changes stay within `PongGame/` and `alpha_engine/` — no other game projects touched.
- No engine-level changes to `AEGraphics.cpp` or any existing `AE*` module — `AEParticle` is additive.
- Web and native builds share the same source (the one-source-two-targets rule).
- `AEParticle.cpp` must be added to the Pong web CMakeLists and to the VS project.
- `rand()` is used for particle velocity randomization (no platform divergence needed).
- Win text strings are hardcoded as UTF-8 literals; the Arial_Italic.ttf font already loaded by `GameStartup` is used.

---

## Excluded (out of scope)

- Sound effects on scoring/win
- Difficulty progression beyond ball speed (no NPC speed scaling)
- Animated win screen (text appears immediately, no fade-in)
- Particle effects for paddles
