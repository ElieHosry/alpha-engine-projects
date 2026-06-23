# Project Handoff — Alpha Engine Platformer (native + WebAssembly)

**Read this for orientation: what the project is, where it stands, and how it got
here.** For *how to build, run, and safely change* the code (commands, rules,
architecture, gotchas), see **[`CLAUDE.md`](CLAUDE.md)**. The full architecture
write-up is **[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)** (SAD), and a possible
WebXR effort is scoped in **[`docs/VR_3D_REWORK_PLAN.md`](docs/VR_3D_REWORK_PLAN.md)**.

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

## Key design decisions (why it looks the way it does)

- **SDL2** is the platform layer for both targets: native (real SDL2 in
  `alpha_engine/extern/SDL2`) and web (Emscripten's `-sUSE_SDL=2` port). Same code path.
- Web rendering = **WebGL/GLES2**; textures via **SDL_image**; fonts via the
  **FreeType** Emscripten port. (Native uses GLEW + GDI+.)
- **Per-game web build**: shared CMake in `cmake/`, engine target in
  `alpha_engine/CMakeLists.txt`, and a per-game `web/` folder.
- **Fixed-timestep accumulator** in `Main.cpp` so object speed is identical at any
  frame rate (web ~60Hz vs native uncapped). Input is sampled **per fixed step**.
- Win32-isms needed on web live in `alpha_engine/include/AEWinShim.h`.

*(For how these decisions translate into the code layout and the exact divergence
points, see [`CLAUDE.md`](CLAUDE.md) and [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).)*

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
- Possible **WebXR / 3D** direction — scoped (with a skeptical eye) in
  [`docs/VR_3D_REWORK_PLAN.md`](docs/VR_3D_REWORK_PLAN.md).

## How to resume in a new Claude session

Grant Claude access to this `AE` folder, then say:
> "Continuing the WebAssembly port of this C++ engine. Read HANDOFF.md and
> CLAUDE.md, then help me with <task>."

`CLAUDE.md` is loaded automatically and carries the build commands, the rules
(one source / two targets), and the gotchas.
