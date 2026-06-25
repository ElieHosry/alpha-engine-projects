# Pong Web Port Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a WebAssembly web build for the Pong game so it runs in a browser at `https://ElieHosry.github.io/pong-web/`, with swipe-to-move touch controls on mobile.

**Architecture:** Five new files are created inside a new `PongGame/PongGame/web/` folder — a CMake build definition, two build scripts (Windows + Bash), an HTML shell with swipe gesture handling, and a deploy script. No C++ changes are needed: Pong's `Main.cpp` already contains full `__EMSCRIPTEN__` guards and the `AEGameTick` / `emscripten_set_main_loop` entry point.

**Tech Stack:** Emscripten (emcmake + emcc), CMake 3.20+, Ninja, SDL2/SDL_image/FreeType via Emscripten ports, WebGL/GLES2.

## Global Constraints

- Emscripten SDK must be active in the shell before building (`C:\dev\emsdk\emsdk_env.bat` on Windows, `source .../emsdk_env.sh` on Bash).
- All new files live exclusively under `PongGame/PongGame/web/` — nothing outside that folder is created or modified.
- Native Visual Studio project (`PongGame.vcxproj`) must remain untouched.
- Canvas size: 800×600 (matches `AESysInit(... 800, 600 ...)` in `Main.cpp`).
- AE_ROOT from `web/` is `../../..` (3 levels up: `web → PongGame(inner) → PongGame(outer) → AE`).
- Assets live at `PongGame/Resources/` → `../../Resources` relative to `web/`.

---

## File Map

| File | Action | Responsibility |
|---|---|---|
| `PongGame/PongGame/web/CMakeLists.txt` | Create | CMake build definition for the Pong WASM target |
| `PongGame/PongGame/web/build.bat` | Create | Windows build script (Debug / Release / Profile / run / clean) |
| `PongGame/PongGame/web/build.sh` | Create | Bash build script for Git Bash / WSL |
| `PongGame/PongGame/web/shell.html` | Create | Browser HTML shell — Pong branding + swipe touch controls |
| `PongGame/PongGame/web/redeploy.bat` | Create | Release build + publish to `pong-web` GitHub Pages repo |

---

## Task 1: CMakeLists.txt + Build Scripts

**Files:**
- Create: `PongGame/PongGame/web/CMakeLists.txt`
- Create: `PongGame/PongGame/web/build.bat`
- Create: `PongGame/PongGame/web/build.sh`

**Interfaces:**
- Produces: `web/out/Pong.html`, `Pong.js`, `Pong.wasm`, `Pong.data` (consumed by Task 3 shell and Task 4 deploy)

- [ ] **Step 1: Create the web/ directory**

In Explorer or terminal, create the folder:
```
PongGame\PongGame\web\
```

- [ ] **Step 2: Write CMakeLists.txt**

Create `PongGame/PongGame/web/CMakeLists.txt` with this exact content:

```cmake
# ===========================================================================
#  PongGame - WEB (Emscripten) build
#  Produces Pong.html / .js / .wasm / .data from this game's sources
#  plus the shared Alpha Engine.
#
#  Build:   build.bat [Release]   (Windows, requires active emsdk)
#           ./build.sh [Release]  (Git Bash / WSL)
#  The native Visual Studio project (PongGame.vcxproj) is untouched.
# ===========================================================================

cmake_minimum_required(VERSION 3.20)
project(Pong_Web LANGUAGES C CXX)

# Repo root: web/ sits 3 levels below it
#   web -> PongGame (inner) -> PongGame (outer) -> AE
get_filename_component(AE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../.." ABSOLUTE)

# Shared build modules.
include("${AE_ROOT}/cmake/EmscriptenCommon.cmake")
include("${AE_ROOT}/cmake/AEWebGame.cmake")

# Shared engine (compiled into this build tree).
add_subdirectory("${AE_ROOT}/alpha_engine" "${CMAKE_BINARY_DIR}/alpha_engine")

# This game's translation units.
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

- [ ] **Step 3: Write build.bat**

Create `PongGame/PongGame/web/build.bat` with this exact content:

```bat
@echo off
REM ===========================================================================
REM  Build the Pong web target (Emscripten) on Windows.
REM
REM  Prerequisites (see AE/AE/WEB_BUILD_SETUP.md):
REM    - Emscripten SDK activated in THIS shell:   <emsdk>\emsdk_env.bat
REM    - CMake and Ninja on PATH
REM
REM  Usage:
REM    build.bat            Debug build into .\out
REM    build.bat Release    Release build
REM    build.bat run        Debug build, then serve with emrun
REM    build.bat clean      Delete the .\out build folder
REM ===========================================================================
setlocal

set "HERE=%~dp0"
set "BUILD_DIR=%HERE%out"
set "BUILD_TYPE=Debug"
set "DO_RUN=0"

set "CMAKE_EXTRA="
if /I "%~1"=="Release" set "BUILD_TYPE=Release"
if /I "%~1"=="run"     set "DO_RUN=1"
if /I "%~1"=="Profile" (
    REM Optimized build that keeps wasm function names for the DevTools profiler.
    set "BUILD_TYPE=Release"
    set "CMAKE_EXTRA=-DAE_PROFILE=ON"
)
if /I "%~1"=="clean" (
    echo Removing "%BUILD_DIR%" ...
    if exist "%BUILD_DIR%" rmdir /S /Q "%BUILD_DIR%"
    echo Done.
    exit /b 0
)

REM --- Check the Emscripten toolchain is active --------------------------------
where emcc >nul 2>nul
if errorlevel 1 (
    echo ERROR: 'emcc' not found on PATH.
    echo Activate the Emscripten SDK in this shell first, e.g.:
    echo     C:\dev\emsdk\emsdk_env.bat
    exit /b 1
)

where emcmake >nul 2>nul
if errorlevel 1 (
    echo ERROR: 'emcmake' not found ^(Emscripten SDK not activated^).
    exit /b 1
)

where ninja >nul 2>nul
if errorlevel 1 (
    echo ERROR: 'ninja' not found on PATH. Install Ninja, e.g.:
    echo     winget install Ninja-build.Ninja
    exit /b 1
)

REM --- Configure + build -------------------------------------------------------
echo.
echo === Configuring (%BUILD_TYPE%) ===
call emcmake cmake -S "%HERE%." -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%BUILD_TYPE% %CMAKE_EXTRA%
if errorlevel 1 (
    echo CMake configure FAILED.
    exit /b 1
)

echo.
echo === Building ===
call cmake --build "%BUILD_DIR%"
if errorlevel 1 (
    echo Build FAILED.
    exit /b 1
)

echo.
echo Build complete: "%BUILD_DIR%\Pong.html"

if "%DO_RUN%"=="1" (
    echo Launching emrun ...
    call emrun "%BUILD_DIR%\Pong.html"
) else (
    echo Run it with:  emrun "%BUILD_DIR%\Pong.html"
)

endlocal
```

- [ ] **Step 4: Write build.sh**

Create `PongGame/PongGame/web/build.sh` with this exact content:

```bash
#!/usr/bin/env bash
# ===========================================================================
#  Build the Pong web target (Emscripten).
#
#  Prerequisite: an active Emscripten SDK environment, e.g.
#      source /path/to/emsdk/emsdk_env.sh
#
#  Usage:
#      ./build.sh            # Debug build into ./out
#      ./build.sh Release    # Release build
# ===========================================================================
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_TYPE="${1:-Debug}"
BUILD_DIR="$HERE/out"

if ! command -v emcmake >/dev/null 2>&1; then
  echo "ERROR: 'emcmake' not found. Activate the Emscripten SDK first:" >&2
  echo "       source /path/to/emsdk/emsdk_env.sh" >&2
  exit 1
fi

emcmake cmake -S "$HERE" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
cmake --build "$BUILD_DIR" -j

echo
echo "Build complete: $BUILD_DIR/Pong.html"
echo "Run it with:    emrun \"$BUILD_DIR/Pong.html\""
```

- [ ] **Step 5: Verify the build works**

Open a shell with emsdk active, `cd` into `PongGame/PongGame/web/`, and run:

```bat
build.bat Release
```

Expected output (last lines):
```
Build complete: "...\web\out\Pong.html"
Run it with:  emrun "...\web\out\Pong.html"
```

Verify these four files exist in `web/out/`:
- `Pong.html`
- `Pong.js`
- `Pong.wasm`
- `Pong.data`

If the build fails with a CMake configure error, double-check that emsdk is active (`emcc --version` should print a version number).

- [ ] **Step 6: Smoke-test in the browser**

```bat
emrun out\Pong.html
```

The browser should open, the canvas should appear, and the game should run (paddles visible, ball moving). Keyboard W/S should move the left paddle. Audio will be silent (expected — FMOD is stubbed on web).

- [ ] **Step 7: Commit**

```bat
git add PongGame/PongGame/web/CMakeLists.txt PongGame/PongGame/web/build.bat PongGame/PongGame/web/build.sh
git commit -m "feat(pong): add Emscripten web build (CMakeLists + build scripts)"
```

---

## Task 2: shell.html — Pong Branding + Swipe Controls

**Files:**
- Create: `PongGame/PongGame/web/shell.html`

**Interfaces:**
- Consumes: `out/Pong.js` (injected via `{{{ SCRIPT }}}` by Emscripten at link time)
- Produces: the browser page served to players; swipe gesture maps to W/S synthetic keyboard events consumed by SDL's keyboard state

- [ ] **Step 1: Create shell.html**

Create `PongGame/PongGame/web/shell.html` with this exact content:

```html
<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no, viewport-fit=cover">
  <title>Pong (Alpha Engine / WebGL)</title>
  <link rel="icon" href="data:,">
  <!-- iOS "Add to Home Screen": launches chromeless (no Safari toolbars) -->
  <meta name="apple-mobile-web-app-capable" content="yes">
  <meta name="apple-mobile-web-app-status-bar-style" content="black-translucent">
  <meta name="apple-mobile-web-app-title" content="Pong">
  <meta name="mobile-web-app-capable" content="yes">
  <style>
    html, body { margin: 0; height: 100%; background: #000; color: #ddd;
                 font-family: system-ui, sans-serif; overflow: hidden; }
    /* Fill the whole viewport and center the canvas. 100dvh tracks the live
       visible height on mobile (toolbars / orientation changes). */
    .wrap { display: flex; align-items: center; justify-content: center;
            width: 100vw; height: 100vh; height: 100dvh; padding: 0; box-sizing: border-box; }
    /* Backing buffer stays 800x600; fit inside BOTH width and height so the 4:3
       game letterboxes instead of overflowing off-screen in landscape. */
    canvas { background: #000; display: block; image-rendering: pixelated; outline: none;
             max-width: 100vw; max-height: 100vh; max-height: 100dvh;
             touch-action: none; /* hand all pointer events to our JS handler */ }
    /* Loading text/bar overlay - taken out of layout so they don't shrink the canvas. */
    #status   { position: fixed; top: 8px; left: 0; right: 0; text-align: center;
                font-size: 14px; color: #9a9a9a; pointer-events: none; z-index: 5; }
    progress  { position: fixed; top: 30px; left: 50%; transform: translateX(-50%);
                width: 320px; max-width: 70vw; z-index: 5; }
  </style>
</head>
<body>
  <div class="wrap">
    <div id="status">Loading…</div>
    <progress id="progress" value="0" max="100"></progress>
    <!-- The engine requests an 800x600 window in AESysInit -->
    <canvas id="canvas" width="800" height="600" tabindex="-1"
            oncontextmenu="event.preventDefault()"></canvas>
  </div>

  <script>
    var statusEl = document.getElementById('status');
    var progressEl = document.getElementById('progress');
    var Module = {
      canvas: (function () { return document.getElementById('canvas'); })(),
      print:    function (text) { console.log(text); },
      printErr: function (text) { console.error(text); },
      setStatus: function (text) {
        statusEl.textContent = text;
        if (!text) progressEl.style.display = 'none';
      },
      monitorRunDependencies: function () {}
    };
    Module.setStatus('Downloading…');
    window.onerror = function () { Module.setStatus('Exception thrown - see console'); };
  </script>

  <script>
    // ---- Swipe-to-move touch controls ----
    // On mobile, drag up/down on the canvas to move the left paddle (W / S keys).
    // We dispatch synthetic KeyboardEvent so SDL's keyboard state updates exactly
    // as if the user pressed a real key — no C++ changes needed.
    (function () {
      var canvas = document.getElementById('canvas');
      var activeKey = null; // 'w' | 's' | null — currently held synthetic key
      var lastY = 0;
      var tracking = false;

      var W_DEF = { key: 'w', code: 'KeyW', keyCode: 87 };
      var S_DEF = { key: 's', code: 'KeyS', keyCode: 83 };

      function dispatchKey(type, def) {
        var ev = new KeyboardEvent(type, {
          key: def.key, code: def.code,
          keyCode: def.keyCode, which: def.keyCode,
          bubbles: true, cancelable: true
        });
        window.dispatchEvent(ev); // SDL registers its keyboard listener on window
      }

      // Transition to a new held key (or null = release all).
      // Only dispatches events when the state actually changes.
      function setKey(newKey) {
        if (newKey === activeKey) return;
        if (activeKey === 'w') dispatchKey('keyup',   W_DEF);
        if (activeKey === 's') dispatchKey('keyup',   S_DEF);
        activeKey = newKey;
        if (newKey  === 'w') dispatchKey('keydown', W_DEF);
        if (newKey  === 's') dispatchKey('keydown', S_DEF);
      }

      canvas.addEventListener('pointerdown', function (e) {
        e.preventDefault();
        canvas.setPointerCapture(e.pointerId); // track finger even outside canvas
        lastY = e.clientY;
        tracking = true;
      });

      canvas.addEventListener('pointermove', function (e) {
        if (!tracking) return;
        var deltaY = e.clientY - lastY;
        lastY = e.clientY;
        // Dead zone of ±4px prevents jitter when finger is nearly stationary.
        // Within the dead zone we keep the current direction rather than releasing,
        // so the paddle doesn't stutter during slow/imprecise drags.
        if      (deltaY < -4) setKey('w');
        else if (deltaY >  4) setKey('s');
        // else: inside dead zone — hold current state
      });

      canvas.addEventListener('pointerup',     function () { tracking = false; setKey(null); });
      canvas.addEventListener('pointercancel', function () { tracking = false; setKey(null); });
    })();
  </script>
  {{{ SCRIPT }}}
</body>
</html>
```

- [ ] **Step 2: Rebuild with the shell**

The CMakeLists.txt already references `shell.html` via the `SHELL` argument, so a
plain rebuild picks it up. Run:

```bat
build.bat Release
```

- [ ] **Step 3: Test swipe on a mobile device (or DevTools)**

Serve over the network (replace `<PC-IP>` with your machine's local IP):

```bat
emrun --hostname 0.0.0.0 --port 8080 --no_browser out\Pong.html
```

Then open `http://<PC-IP>:8080/Pong.html` on a phone.

Verify:
- Dragging a finger **upward** on the canvas moves the left paddle up.
- Dragging a finger **downward** moves the left paddle down.
- Lifting the finger stops the paddle.
- The AI (right paddle) still moves automatically.
- On desktop, W/S keyboard still works normally.

Alternatively, open Chrome DevTools → Toggle device toolbar → simulate touch.

- [ ] **Step 4: Commit**

```bat
git add PongGame/PongGame/web/shell.html
git commit -m "feat(pong): add browser shell with Pong branding and swipe touch controls"
```

---

## Task 3: redeploy.bat — GitHub Pages Deployment

**Files:**
- Create: `PongGame/PongGame/web/redeploy.bat`

**Interfaces:**
- Consumes: `web/out/Pong.html` + `Pong.js` + `Pong.wasm` + `Pong.data` (produced by Task 1)
- Produces: live deployment at `https://ElieHosry.github.io/pong-web/`

- [ ] **Step 1: Create the pong-web GitHub repo (one-time manual step)**

1. Go to `https://github.com/new` and create a repo named **`pong-web`** (public).
2. Go to Settings → Pages → Source: **Deploy from a branch** → Branch: `main`, folder `/` (root).
3. Clone the empty repo locally, e.g.:
   ```
   git clone https://github.com/ElieHosry/pong-web C:\Users\elieh\Desktop\pong-web
   ```
   Note the local clone path — you'll put it in `REPO_DIR` below.

- [ ] **Step 2: Write redeploy.bat**

Create `PongGame/PongGame/web/redeploy.bat` with this exact content.
**Before saving**, replace the placeholder path in `REPO_DIR` with the actual path
where you cloned `pong-web` in Step 1.

```bat
@echo off
REM ===========================================================================
REM  redeploy.bat - one-command publish to GitHub Pages (pong-web).
REM
REM  Does: Release build -> copy out\ into your Pages repo (Pong.html
REM        becomes index.html, ensures .nojekyll) -> git add/commit/push.
REM
REM  ONE-TIME SETUP: set REPO_DIR below to your local clone of the pong-web
REM  repo (the folder you ran `git clone https://github.com/ElieHosry/pong-web`).
REM
REM  Usage:
REM    redeploy.bat                       build + deploy (auto commit message)
REM    redeploy.bat "fixed paddle bug"    build + deploy with a commit message
REM    redeploy.bat nobuild               skip the build, just redeploy out\
REM ===========================================================================
setlocal EnableExtensions

REM ---- EDIT THIS to your pong-web Pages repo's local folder -----------------
set "REPO_DIR=C:\REPLACE\WITH\YOUR\LOCAL\PATH\TO\pong-web"
REM ---------------------------------------------------------------------------

set "HERE=%~dp0"
set "OUT=%HERE%out"
set "DO_BUILD=1"
set "MSG=%~1"

if /I "%~1"=="nobuild" ( set "DO_BUILD=0" & set "MSG=%~2" )
if "%MSG%"=="" set "MSG=update web build"

REM ---- sanity checks ---------------------------------------------------------
if not exist "%REPO_DIR%\" (
    echo ERROR: REPO_DIR not found: "%REPO_DIR%"
    echo Edit redeploy.bat and set REPO_DIR to your local pong-web repo folder.
    exit /b 1
)
if not exist "%REPO_DIR%\.git\" (
    echo ERROR: "%REPO_DIR%" is not a git repository ^(no .git folder^).
    exit /b 1
)
where git >nul 2>nul || ( echo ERROR: git not found on PATH. & exit /b 1 )

REM ---- build (Release) -------------------------------------------------------
if "%DO_BUILD%"=="1" (
    echo === Building Release ===
    call "%HERE%build.bat" Release
    if errorlevel 1 ( echo Build FAILED - aborting deploy. & exit /b 1 )
)

if not exist "%OUT%\Pong.html" (
    echo ERROR: "%OUT%\Pong.html" not found. Build first ^(or run without nobuild^).
    exit /b 1
)

REM ---- copy build output into the repo (Pong.html -> index.html) ------------
echo === Copying build output to "%REPO_DIR%" ===
copy /Y "%OUT%\Pong.html" "%REPO_DIR%\index.html"  >nul || goto :copyfail
copy /Y "%OUT%\Pong.js"   "%REPO_DIR%\Pong.js"     >nul || goto :copyfail
copy /Y "%OUT%\Pong.wasm" "%REPO_DIR%\Pong.wasm"   >nul || goto :copyfail
if exist "%OUT%\Pong.data" copy /Y "%OUT%\Pong.data" "%REPO_DIR%\Pong.data" >nul
if not exist "%REPO_DIR%\.nojekyll" type nul > "%REPO_DIR%\.nojekyll"

REM ---- commit + push ---------------------------------------------------------
pushd "%REPO_DIR%"
git add -A
git diff --cached --quiet
if errorlevel 1 (
    echo === Committing + pushing ===
    git commit -m "%MSG%"  || ( popd & echo Commit FAILED. & exit /b 1 )
    git push               || ( popd & echo Push FAILED ^(check auth / run 'git pull' first^). & exit /b 1 )
    echo.
    echo Deployed. GitHub Pages will update in ~1 minute.
    echo Live URL: https://ElieHosry.github.io/pong-web/
) else (
    echo Nothing changed since last deploy - skipping commit/push.
)
popd

endlocal
exit /b 0

:copyfail
echo ERROR: failed to copy build files to "%REPO_DIR%".
exit /b 1
```

- [ ] **Step 3: Run redeploy.bat**

```bat
redeploy.bat "initial Pong web deployment"
```

Expected output (last lines):
```
Deployed. GitHub Pages will update in ~1 minute.
Live URL: https://ElieHosry.github.io/pong-web/
```

- [ ] **Step 4: Verify the live URL**

Wait ~60 seconds, then open `https://ElieHosry.github.io/pong-web/` in a browser.
The Pong game should load and be playable. Verify on both desktop (keyboard) and
mobile (swipe).

- [ ] **Step 5: Commit**

```bat
git add PongGame/PongGame/web/redeploy.bat
git commit -m "feat(pong): add redeploy.bat for GitHub Pages deployment"
```
