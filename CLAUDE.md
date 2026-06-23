# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What this is

A C++ real-time 2D game engine (`alpha_engine/`) and a platformer game built on it (`Project3_Platformer/`). The same source compiles to **two targets**: native Windows (Visual Studio) and **WebAssembly** (Emscripten).

This file is the operating manual (commands, rules, architecture, gotchas). For project **status and history** — what's done, what's open, and why decisions were made — see [`HANDOFF.md`](HANDOFF.md). The full architecture write-up is [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) (SAD); a possible WebXR effort is scoped in [`docs/VR_3D_REWORK_PLAN.md`](docs/VR_3D_REWORK_PLAN.md).

## The one rule that governs everything

**One source, two targets.** Every web/native divergence is isolated behind `#if defined(__EMSCRIPTEN__)`. The native build is the original code path and must never regress when you touch the web path. Before editing engine code, check whether the function already has an `__EMSCRIPTEN__` branch and preserve the symmetry — the native branch and the web branch should be two implementations of the *same* behavior.

## Build & run

### Web (Emscripten + CMake + Ninja)

Requires the emsdk active **in the current shell** (`C:\dev\emsdk\emsdk_env.bat`; PowerShell uses `.ps1`, Git Bash `source …/emsdk_env.sh`). `emcmake not found` almost always means emsdk isn't active in this shell (or you double-clicked `build.bat` from Explorer, which spawns a fresh one).

```bat
cd Project3_Platformer\Assignment_3_Part_2-Solved\csd1130_Platformer\web
build.bat              REM Debug   -> out\Platformer.html
build.bat Release      REM Optimized — ALWAYS use this for real/perf testing
build.bat Profile      REM Optimized + wasm function names (DevTools profiler)
build.bat run          REM Debug build, then emrun
build.bat clean
emrun out\Platformer.html     REM must be served over HTTP; cannot double-click (.data is fetched)
```

Git Bash / WSL equivalent: `./build.sh [Release]`. Raw CMake: `emcmake cmake -S . -B out -G Ninja -DCMAKE_BUILD_TYPE=Release && cmake --build out`. Profiling manually: add `-DAE_PROFILE=ON`.

Test on a phone over Wi-Fi: `emrun --hostname 0.0.0.0 --port 8080 --no_browser out\Platformer.html`, then open `http://<PC-IP>:8080/Platformer.html`.

### Native (Visual Studio 2022)

Open the VS solution under `alpha_engine/build/mvs2022/` (engine) / `csd1130_Platformer.vcxproj` (game) and build/debug as usual. The native build needs the third-party libs in `alpha_engine/extern/` (SDL2, FMOD, FreeType, GLEW) — see `alpha_engine/extern/SDL2/PLACE_SDL2_HERE.md` for the SDL2 layout. **The VS build never reads any CMakeLists** — those are web-only.

### Deploy (web → GitHub Pages)

Publish `web/out/` (rename `Platformer.html` → `index.html`, add an empty `.nojekyll`). `web/redeploy.bat "msg"` automates it once its `REPO_DIR` points at your Pages clone. Live: https://ElieHosry.github.io/platformer-web/

There are no automated tests in this repo; verification is running the game (native and `build.bat Release` on web).

## Architecture you can't see from one file

**Layers, dependencies point down only:** game (`Project3_Platformer`) → engine `AE*` API (`alpha_engine`) → SDL2 platform layer → backend libs (OpenGL/WebGL, SDL_image/GDI+, FreeType, FMOD). Game code talks to the world *only* through `AE*` functions; the engine only calls back into the game through registered function pointers.

**The game loop lives in `Project3_Platformer/.../Src/Main.cpp`** and has two entry points selected at compile time: native `WinMain` (classic nested `while` loops) and web `AEGameTick` driven by `emscripten_set_main_loop` (the browser owns the clock, so the nested loops are flattened). Both share `AEUpdateFixedSteps()`: a **fixed-timestep accumulator** that advances simulation in `g_fixedDT` ticks from real elapsed time (capped at 5 steps), so motion speed is frame-rate independent. **Input is sampled once per fixed step** (`AEInputUpdate()` inside the accumulator), *not* per rendered frame — sampling per frame erases one-frame "triggered" key edges and drops presses. Don't move it.

**State management is a function-pointer state machine** (`GameStateMgr.{h,cpp}`, `GameStateList.h`). `GameStateMgrUpdate()` rebinds six globals (`GameStateLoad/Init/Update/Draw/Free/Unload`) to the active state; the loop calls them blind. Transitions are requested by writing `gGameStateNext`. The engine/game communicate largely through globals (`g_dt`, `g_appTime`, `gGameState*`, `gAESDLWindow`, …), not passed context.

**Engine subsystems** (`alpha_engine/src/`, API in `include/AEEngine.h`): `AESystem` (lifecycle — `AESysInit` creates the SDL window + GL context then brings up the others; `AESysFrameStart/End`; SDL event pump), `AEGraphics` (sprites/fonts/transforms), `AEInput` (keyboard/mouse edge state, `AEVK_*` codes), `AEAudio` (FMOD), `AEFrameRateController` (timing, `AEGetTime`), and math (`AEMath`, `AEVec2`, `AEMtx33`, plus `Matrix4`/`Vector4`/`Point4` used internally).

**Where the web port actually diverges** (all `#if defined(__EMSCRIPTEN__)`):
- `include/AEWinShim.h` — supplies Win32 types + `VK_*` codes so the unchanged headers compile (web only; native uses real `<windows.h>`).
- `src/AEGraphics.cpp` — GLES2 + `SDL_image` decode on web; GLEW + GDI+ on native. Note: the **shaders are already 4×4** (`uPMatrix * uVMatrix * uWorldMatrix`); the "2D-ness" is in CPU-side matrix construction (ortho projection, `AEMtx33`), not the GPU pipeline.
- `src/AEAudio.cpp` — **no-op stubs on web** (FMOD has no web backend; audio is silent in the browser — known open item).
- `src/AEInput.cpp` / `AESystem.cpp` — SDL keyboard/mouse and web fullscreen via the HTML5 API (must be requested inside a real user gesture).

**Web build system** (`cmake/`): `EmscriptenCommon.cmake` defines the `ae_web_common` INTERFACE target carrying all SDL2/SDL_image/FreeType-port + WebGL flags; `AEWebGame.cmake` provides `ae_add_web_game()` (one call per game → `.html/.js/.wasm/.data`, wires the HTML shell and `--preload-file <assets>@Resources`). `alpha_engine/CMakeLists.txt` builds the engine as a STATIC lib for web. **The web build ignores `extern/` entirely** — SDL2/SDL_image/FreeType come from Emscripten ports, not the native libs.

**Web assets & paths:** use forward-slash, `../Resources/...` paths — they resolve on Windows *and* in the Emscripten virtual FS (assets preloaded under `/Resources`). Each game gets its own `.data` image.

## Gotchas

- Always perf-test the web build with `build.bat Release` (Debug is `-O0` + assertions, far slower). A per-frame `glGetError()` was removed because it stalls WebGL badly — don't reintroduce per-frame GL error checks on the hot path.
- iOS Safari can't fullscreen a canvas; F1/⛶ are no-ops there (use Add to Home Screen). Desktop/Android fullscreen works.
- `legacy/` and `Dep/AlphaEngine - Not used Here/` contain old engine copies — **not** the live code. The live engine is `alpha_engine/src/` + `alpha_engine/include/`.

## Controls

← / → move, Space jump, 1 / 2 / Q (menu: start L1 / L2 / quit), Esc back to menu, F1 fullscreen (desktop). Touch buttons dispatch synthetic `KeyboardEvent`s, so they drive the identical input path as a real keyboard (no C++ changes for touch).
