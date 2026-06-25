# Pong Web Port — Design Spec

**Date:** 2026-06-25
**Status:** Approved

---

## Overview

Port the Pong game to WebAssembly so it can be played in a browser, matching the
Platformer's existing web deployment. No C++ source changes are needed — Pong's
`Main.cpp` already contains full `__EMSCRIPTEN__` support (the `AEGameTick` loop,
`emscripten_set_main_loop`, and the web `main()` entry point). Audio is already
silently stubbed on web via the engine's FMOD no-op path.

The work is purely **build infrastructure**: five new files inside a new `web/`
folder.

---

## File Structure

```
AE/
└── PongGame/
    ├── PongGame/
    │   ├── Src/              (unchanged)
    │   ├── Include/          (unchanged)
    │   └── web/              ← NEW — all new files go here
    │       ├── CMakeLists.txt
    │       ├── build.bat
    │       ├── build.sh
    │       ├── shell.html
    │       └── redeploy.bat
    └── Resources/            (unchanged — preloaded as-is)
```

No files outside `web/` are created or modified. The native Visual Studio project
(`PongGame.vcxproj`) is untouched.

---

## CMakeLists.txt

`web/` sits **3 levels** below `AE/` (Platformer sits 4 levels down), so `AE_ROOT`
uses `../../..` instead of `../../../..`.

```cmake
cmake_minimum_required(VERSION 3.20)
project(Pong_Web LANGUAGES C CXX)

# web/ -> PongGame (inner) -> PongGame (outer) -> AE  (3 levels up)
get_filename_component(AE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

include("${AE_ROOT}/cmake/EmscriptenCommon.cmake")
include("${AE_ROOT}/cmake/AEWebGame.cmake")

add_subdirectory("${AE_ROOT}/alpha_engine" "${CMAKE_BINARY_DIR}/alpha_engine")

file(GLOB PONG_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../Src/*.cpp")

ae_add_web_game(
  NAME      Pong
  SOURCES   ${PONG_SRC}
  INCLUDES  "${CMAKE_CURRENT_SOURCE_DIR}/../Include"
  ASSETS    "${CMAKE_CURRENT_SOURCE_DIR}/../../Resources"
  SHELL     "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
)
```

Output: `web/out/Pong.html` + `Pong.js` + `Pong.wasm` + `Pong.data`.
Assets from `PongGame/Resources/` are preloaded at `/Resources` in the Emscripten
virtual filesystem, matching the `../Resources/...` paths already in the game code.

---

## build.bat

Identical structure to the Platformer's `build.bat`. Supports the same modes:

| Command              | Effect                                      |
|----------------------|---------------------------------------------|
| `build.bat`          | Debug build → `out/Pong.html`               |
| `build.bat Release`  | Optimized — use for real / perf testing     |
| `build.bat Profile`  | Release + wasm function names (DevTools)    |
| `build.bat run`      | Debug build, then `emrun out\Pong.html`     |
| `build.bat clean`    | Delete `out/`                               |

Checks that `emcc`, `emcmake`, and `ninja` are on `PATH` before attempting to build.

---

## build.sh

Git Bash / WSL equivalent of `build.bat`. Same modes, same guards.

---

## shell.html

Follows the Platformer shell's style exactly:

- Title: `Pong (Alpha Engine / WebGL)`
- iOS home-screen meta tags with name `Pong`
- Same black background, centered 800×600 canvas, `image-rendering: pixelated`
- Same loading status / progress bar overlay
- Same `Module` JS object setup

**Touch controls — swipe gesture (no visible buttons):**

A `pointermove` listener on the canvas converts drag direction into held W / S
key state. The engine reads `AEInputCheckCurr(AEVK_W/S)` which expects a
continuously held key, so we dispatch `keydown` / `keyup` on direction change:

| Gesture                        | Synthetic key events                 |
|--------------------------------|--------------------------------------|
| Finger moves up (deltaY < −4px) | `keydown W` + `keyup S`            |
| Finger moves down (deltaY > 4px) | `keydown S` + `keyup W`           |
| Finger lifts (`pointerup`)     | `keyup W` + `keyup S`              |

Dead zone of ±4px prevents jitter when the finger is nearly stationary.
Direction state is tracked so `keydown` is only dispatched once per direction
change, not on every `pointermove` event.

`pointerdown` captures the pointer (`setPointerCapture`) so movement outside the
canvas still registers.

Esc (menu / quit) has no touch equivalent — keyboard-only on mobile. Acceptable
because Pong has only one game state (no menu to navigate).

---

## redeploy.bat

Same structure as the Platformer's `redeploy.bat`:

1. Runs `build.bat Release` to produce a fresh `out/`
2. Copies `out/` into the local clone of the `pong-web` GitHub Pages repo
3. Renames `Pong.html` → `index.html`
4. Adds an empty `.nojekyll` marker
5. Commits with a provided message and pushes

The `REPO_DIR` variable at the top of the file must be set to the local path of
the `pong-web` repo clone before first use.

**GitHub Pages setup required (one-time, manual):**
- Create a new GitHub repo named `pong-web`
- Enable GitHub Pages (Settings → Pages → Source: `main` branch, root `/`)
- Clone it locally and set `REPO_DIR` in `redeploy.bat`
- Live URL: `https://ElieHosry.github.io/pong-web/`

---

## What is NOT changing

- `PongGame/PongGame/Src/*.cpp` — no modifications
- `PongGame/PongGame/Include/*.h` — no modifications
- `PongGame/PongGame/PongGame.vcxproj` — no modifications
- `alpha_engine/` — no modifications
- `cmake/` — no modifications
- Platformer web build — no modifications
