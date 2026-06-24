# PongGame — Project Reference

Complete context for picking up this project in any future session.
Companion to the repo-wide [`CLAUDE.md`](../CLAUDE.md) and [`HANDOFF.md`](../HANDOFF.md).

---

## File map

```
AE/
├── alpha_engine/                  ← engine (DO NOT edit casually)
│   ├── build/mvs2022/alpha_engine.vcxproj   GUID: {ff4da28e-...}
│   ├── bin/engine/x64/            Alpha_Engine.lib / .dll (Release)
│   │                              Alpha_EngineD.lib / .dll (Debug)
│   └── extern/                    freetype, fmod, SDL2 (native-only)
│
└── PongGame/
    ├── PONG.md                    ← this file
    ├── PongGame/
    │   ├── PongGame.vcxproj       GUID: {955D0A3F-...}
    │   ├── Include/
    │   │   ├── GameStateList.h    enum { GS_PONG=0, GS_RESTART, GS_QUIT }
    │   │   ├── GameStateMgr.h     state globals + 6 fn-ptr globals
    │   │   ├── GameState_Pong.h   AABB, Paddle, Ball structs + 6 state fns
    │   │   └── Main.h             extern gFontId, g_fixedDT, g_dt, g_appTime
    │   └── Src/
    │       ├── Main.cpp           WinMain + Emscripten entry, fixed-step loop
    │       ├── GameStateMgr.cpp   GameStateMgrUpdate() wires GS_PONG
    │       └── GameState_Pong.cpp ALL game logic — paddles, ball, AI, drawing
    └── Resources/
        ├── Fonts/Arial_Italic.ttf
        └── Textures/DigiPen_Logo.png   (505×122, white on transparent PNG)
```

**Output layout** (post-build copies everything here):
```
PongGame/bin/
├── Release-x64/PongGame.exe
├── Debug-x64/PongGameD.exe
└── Resources/           ← xcopy'd from PongGame/Resources/ with /s
    ├── Fonts/
    └── Textures/
```

---

## Build

### Visual Studio 2022 (native)

Open `PongGame/PongGame/PongGame.vcxproj` directly — no `.sln` needed.
Build order: the `<ProjectReference>` to `alpha_engine.vcxproj` ensures the
engine builds first automatically.

```
MSBuild PongGame\PongGame\PongGame.vcxproj /p:Configuration=Release /p:Platform=x64
```

### Key vcxproj facts

| Property | Value |
|---|---|
| `$(AERoot)` | `$(MSBuildProjectDirectory)\..\..\alpha_engine\` |
| `$(OutDir)` | `..\bin\$(Configuration)-$(Platform)\` |
| Debug lib | `Alpha_EngineD.lib` → links `Alpha_EngineD.dll` |
| Release lib | `Alpha_Engine.lib` → links `Alpha_Engine.dll` |
| IncludePath | `..\..\alpha_engine\include;Include;...` |
| LibraryPath | `..\..\alpha_engine\bin\engine\x64;...` |

Post-build copies Resources + DLLs (freetype, fmod, Alpha_Engine, SDL2) to `$(OutDir)`.

### Launch from PowerShell

```powershell
Start-Process "PongGame\bin\Release-x64\PongGame.exe"
```

**CWD is automatically pinned** — `WinMain` calls `GetModuleFileNameW` +
`SetCurrentDirectoryW` so `../Resources/...` paths always resolve to the exe's
directory regardless of how the process is launched.

---

## Architecture

### Globals defined in Main.cpp, declared extern in Main.h

```cpp
s8  gFontId;               // AEGfxCreateFont result — use for AEGfxPrint
f64 g_fixedDT = 1.0/60.0; // fixed step size
f64 g_dt      = g_fixedDT; // current step (always == g_fixedDT in practice)
f64 g_appTime = 0.0;       // accumulated simulation time
```

These are **game-side**, not in the engine headers. Any new .cpp that needs
timing must `#include "Main.h"`.

### Fixed-timestep loop

`AEUpdateFixedSteps()` in Main.cpp:
- Caps frame time to 0.25 s (prevents spiral of death)
- Runs at most 5 fixed steps per rendered frame
- Calls `AEInputUpdate()` **inside** the accumulator — input is sampled once
  per fixed step, not per frame. Moving it outside drops one-frame key edges.
- `GameStateDraw()` is called once per rendered frame, outside the accumulator.

### State machine

`GameStateMgrUpdate()` in GameStateMgr.cpp sets 6 function-pointer globals:

```cpp
GameStateLoad   = GameState_PongLoad;
GameStateInit   = GameState_PongInit;
GameStateUpdate = GameState_PongUpdate;
GameStateDraw   = GameState_PongDraw;
GameStateFree   = GameState_PongFree;
GameStateUnload = GameState_PongUnload;
```

Transition: write `gGameStateNext = GS_QUIT` (or any state) and the outer
loop handles Free/Unload/swap on the next iteration.

---

## Screen coordinate system

Window: 800 × 600.

```
AEGfxGetWinMaxX() = 400    (right edge)
AEGfxGetWinMaxY() = 300    (top edge)
Origin (0,0) = center of screen
+X = right,  +Y = up
```

---

## Gameplay constants

```cpp
PADDLE_HALF_W  =   8.0f   // paddle half-width  (8px wide total → 16px)
PADDLE_HALF_H  =  50.0f   // paddle half-height (100px tall)
BALL_HALF_SIZE =   8.0f   // ball is a square, 16×16
PADDLE_INSET   = 360.0f   // X distance from center to paddle face
BALL_SPEED     = 250.0f   // pixels/second
PLAYER_SPEED   = 300.0f   // left paddle (W/S keys)
AI_SPEED       = 220.0f   // right paddle AI (slightly slower → beatable)
```

Initial ball velocity (~20° incline heading right):
```cpp
vel = { BALL_SPEED * 0.94f, BALL_SPEED * 0.34f }
```

### Controls

| Key | Action |
|---|---|
| W | Left paddle up |
| S | Left paddle down |
| Esc | Quit |
| F1 | Fullscreen toggle (native only) |

Right paddle is autonomous AI — moves toward `sBall.pos.y` at `AI_SPEED`.

---

## Rendering pipeline

All game objects share **one unit quad mesh** (`sMesh`) created in `Load`.
`DrawRect(cx, cy, hw, hh)` scales and translates it each call.

### Color rendering (paddles, ball)

```cpp
AEGfxSetRenderMode(AE_GFX_RM_COLOR);
AEGfxTextureSet(NULL, 0.0f, 0.0f);
AEGfxSetBlendMode(AE_GFX_BM_NONE);
AEGfxSetTransparency(1.0f);
AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
```

Vertex color is the ARGB baked into the mesh (0xFFFFFFFF = white).

### Texture rendering (DigiPen logo)

```cpp
AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
AEGfxTextureSet(sLogoTex, 0.0f, 0.0f);
AEGfxSetBlendMode(AE_GFX_BM_BLEND);
AEGfxSetTransparency(0.7f);          // 70% opacity watermark
AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
```

Draw order: **logo first**, then paddles/ball on top.

### UV layout of the unit quad

```
vertex        position     UV
bottom-left  (-0.5,-0.5)  (0, 1)
bottom-right ( 0.5,-0.5)  (1, 1)
top-left     (-0.5, 0.5)  (0, 0)
top-right    ( 0.5, 0.5)  (1, 0)
```

`(0,0)` = top-left of the loaded image (GDI+ stores rows top-down; OpenGL
texture unit (0,0) is bottom-left in clip-space but the image is flipped on
upload, making v=0 appear at the top visually).

---

## Engine API quick reference

| Call | Notes |
|---|---|
| `AEGfxMeshStart()` / `AEGfxTriAdd(x,y,argb,tu,tv, ...)` / `AEGfxMeshEnd()` | Build mesh. **All 6 values per vertex.** |
| `AEGfxMeshDraw(mesh, AE_GFX_MDM_TRIANGLES)` | Draw using current shader+transform |
| `AEGfxSetTransform(m.m)` | Takes `float[3][3]` from `AEMtx33.m` |
| `AEGfxTextureLoad(path)` | Returns `AEGfxTexture*`; returns NULL on failure (silent) |
| `AEGfxTextureUnload(tex)` | Free before exiting state |
| `AEGfxSetTransparency(f)` | Multiplies fragment alpha in shader |
| `AEGfxPrint(fontId, str, x, y, scale, r, g, b, alpha)` | **9 args** — easy to forget the last `alpha` |
| `AEInputCheckCurr(key)` | Held — use for movement |
| `AEInputCheckTriggered(key)` | One-shot press — use for actions |
| `AEClamp(val, min, max)` | Float clamp from engine math |
| `AEGfxGetWinMaxX/Y()` | Half-extents of the window |

---

## Hard-won gotchas

### 1. Working directory — textures & fonts

`AEGfxTextureLoad` and `AEGfxCreateFont` resolve paths relative to CWD.
When launched via `Start-Process` (PowerShell) the CWD is the **shell's**
directory, not the exe directory — so `../Resources/...` resolves to a
non-existent path and the call returns NULL **silently**.

**Fix already in place:** `WinMain` calls `SetCurrentDirectoryW` to the
exe directory before `AESysInit`. Never remove this block.

### 2. Texture UV coordinates

`AEGfxTriAdd` takes `(tu, tv)` per vertex. If you pass `0, 0` for all
vertices every fragment samples the same single texel — the entire texture
appears as one pixel (usually transparent corner → invisible quad).
Map UVs across `[0,1]×[0,1]` as shown in the UV layout table above.

### 3. Input function name

The function is `AEInputCheckCurr`, **not** `AEInputCheckCurrent`.
(`AEInputCheckTriggered`, `AEInputCheckPrev`, `AEInputCheckReleased` complete the set.)

### 4. AEGfxPrint argument count

```cpp
AEGfxPrint(gFontId, "text", x, y, scale, r, g, b, 1.0f);
//                                                 ^^^^^ alpha — required 9th arg
```

### 5. Set ALL shader uniforms after SetRenderMode

On the very first call to `SetRenderMode(TEXTURE)`, the TEXTURE shader
program's uniforms are at OpenGL defaults (0 for everything).
Always call `SetTransparency`, `SetColorToMultiply`, and `SetColorToAdd`
after switching modes — never assume the previous frame's values carried over.

### 6. Build order without a .sln

MSBuild doesn't know to build `alpha_engine` first unless the `<ProjectReference>`
is present. The `ReferenceOutputAssembly=false` + `LinkLibraryDependencies=false`
pair gives build-order dependency **without** auto-linking (we link the lib
manually via `AdditionalDependencies`).

### 7. AEGfxSetBlendMode(AE_GFX_BM_NONE) ≠ transparent

`BM_NONE` disables OpenGL blending entirely — transparent texture pixels
(alpha=0) write their **RGB** directly to the framebuffer (black for a white
PNG with transparent BG). Use `BM_BLEND` for PNG transparency.

---

## Extending the game

### Add a score display

```cpp
// In Draw, after switching to COLOR mode:
char buf[32];
snprintf(buf, sizeof(buf), "%d  %d", scoreL, scoreR);
AEGfxPrint(gFontId, buf, -30.0f, 250.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
```

Track `scoreL`/`scoreR` as static ints in GameState_Pong.cpp; increment
instead of just calling `ResetBall()` when the ball exits the screen.

### Add a center divider

Draw a series of small rectangles along x=0 with y stepped by ~20px.

### New game state (e.g. GS_MENU)

1. Add `GS_MENU` before `GS_RESTART` in `GameStateList.h`
2. Add `GameState_Menu{Load/Init/Update/Draw/Free/Unload}` functions
3. Wire them in `GameStateMgrUpdate()` with a new `case GS_MENU:`
4. Change `GameStateMgrInit(GS_MENU)` in `GameStartup()`
