# Project Handoff — Alpha Engine Platformer (native + WebAssembly)

**Read this first.** It orients a new Claude session (or a teammate) on another
machine. For full detail, read the dated/step docs listed below.

---

## What this project is

- `alpha_engine/` — a C++ real-time 2D engine (OpenGL/Win32, built with Visual Studio).
- `Project3_Platformer/` — a platformer game built on the engine.
- It was **ported to run in a web browser as WebAssembly** (via Emscripten),
  while the **native Visual Studio build was kept working the whole time**.

## Current status (as of 2026-06-05)

- ✅ Native Windows build: unchanged, still builds/debugs in Visual Studio.
- ✅ Web build: compiles to `.wasm`, runs at a smooth **60 FPS**.
- ✅ Playable on desktop (keyboard) and **iPhone** (on-screen touch controls).
- ✅ Deployed to **GitHub Pages**: https://ElieHosry.github.io/platformer-web/
- ⚠️ **Audio is silent on web** (FMOD has no web backend; API is stubbed). Open item.
- The whole port is guarded with `#if defined(__EMSCRIPTEN__)`, so nothing changes
  natively.

## Key design decisions

- **SDL2** is the platform layer: native (real SDL2 in `alpha_engine/extern/SDL2`)
  and web (Emscripten's `-sUSE_SDL=2` port). Same code path.
- Web rendering = **WebGL/GLES2**; textures via **SDL_image**; fonts via the
  **FreeType** Emscripten port.
- **Per-game web build**: shared CMake in `cmake/`, engine target in
  `alpha_engine/CMakeLists.txt`, and a per-game `web/` folder.
- **Fixed-timestep accumulator** in `Main.cpp` so object speed is identical at any
  frame rate (web ~60Hz vs native uncapped). Input is sampled **per fixed step**.
- Win32-isms needed on web live in `alpha_engine/include/AEWinShim.h`.

## Where things live

```
AE/
├── HANDOFF.md                         <- this file
├── cmake/                             shared web build (EmscriptenCommon.cmake, AEWebGame.cmake)
├── alpha_engine/
│   ├── CMakeLists.txt                 reusable engine target (web)
│   ├── include/AEWinShim.h            web-only Win32 type/VK/macro shim
│   ├── src/ ...                       engine (web changes guarded by __EMSCRIPTEN__)
│   └── build/mvs2022/                 native Visual Studio project (unchanged)
└── Project3_Platformer/Assignment_3_Part_2-Solved/csd1130_Platformer/
    ├── csd1130_Platformer.vcxproj     native game project (unchanged)
    ├── Src/ , Include/                game code
    └── web/                           <- the web build for this game
        ├── CMakeLists.txt  shell.html  build.bat  build.sh  redeploy.bat
        ├── WEB_BUILD_SETUP.md          full toolchain + build + deploy guide
        └── out/                        build output (index/Platformer.* + .data)
```

## Build & run (web)

Prereqs: Emscripten SDK (emsdk) + CMake + Ninja. See `web/WEB_BUILD_SETUP.md`.

```bat
:: activate emsdk in this shell first:
C:\dev\emsdk\emsdk_env.bat

cd Project3_Platformer\Assignment_3_Part_2-Solved\csd1130_Platformer\web
build.bat Release            :: -> out\Platformer.html  (use Release for real testing)
emrun out\Platformer.html    :: run (cannot just double-click; needs a server)
```
- `build.bat Profile` = optimized + wasm function names (for the DevTools profiler).
- Local Wi-Fi test on a phone: `emrun --hostname 0.0.0.0 --port 8080 --no_browser out\Platformer.html`, then open `http://<PC-IP>:8080/Platformer.html`.

## Deploy (GitHub Pages)

Publish the contents of `web/out/` (rename `Platformer.html` -> `index.html`,
add an empty `.nojekyll`). Repo: `platformer-web` (public). Re-publish with
`web/redeploy.bat "message"` after setting its `REPO_DIR` to your local clone of
the Pages repo. Full steps: `WebBrowser_Platform.docx`.

## Controls

← / → move, Space jump, 1 / 2 / Q (menu: start L1 / L2 / quit), Esc back to menu,
F1 fullscreen (desktop). On touch: the same via on-screen buttons (MENU = Esc, ⛶ = fullscreen).

## Documentation (the "chat", written down)

Read these for the full history and reasoning:

- `2026-06-05.docx` — full session log: every question + detailed solution.
- `Browser_Support.docx` — initial architecture analysis (what porting requires).
- `step1_SDL2Porting.docx` — Step 1: SDL2 backend on native Windows.
- `step2_EMCC_CMAKE_Addition.docx` — Step 2: the emcc/CMake web build.
- `step3_SourceAdaptation.docx` — Step 3: source changes to compile to wasm.
- `step4_WebPort_Debugging.docx` — Step 4: compile fixes + the 60 FPS perf hunt.
- `WebBrowser_Platform.docx` — build + deploy guide (local Wi-Fi and GitHub Pages).

## Suggested next steps / open items

- Add a **web audio backend** (SDL_mixer or OpenAL) to replace the silent stub.
- Optional: widen the camera so widescreen fills the view instead of letterboxing.
- Optional: reduce per-draw uniform uploads for very large levels.

## How to resume in a new Claude session

Grant Claude access to this `AE` folder, then say:
> "Continuing the WebAssembly port of this C++ engine. Read HANDOFF.md and the
> step1–step4 docs, then help me with <task>."
