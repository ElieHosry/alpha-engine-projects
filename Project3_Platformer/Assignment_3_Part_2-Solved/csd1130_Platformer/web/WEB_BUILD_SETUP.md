# Web Build Setup — Platformer (Emscripten / WebAssembly)

How to install the toolchain, build, run, and test the Platformer in a browser
(desktop and iPhone). The native Visual Studio build is unaffected by any of
this — it still uses `csd1130_Platformer.vcxproj` as before. Every web-specific
code change is guarded with `#if defined(__EMSCRIPTEN__)`.

---

## 1. What "Emscripten toolchain" means

**Emscripten** compiles C/C++ into **WebAssembly** (`.wasm`) plus JS glue and an
`.html` host page. Its compiler is `emcc` / `em++`, shipped as the **emsdk**
bundle (its own Clang/LLVM + Node). You do NOT use MSVC for the web build. The
web build is **CMake-driven**, so you also need **CMake** and **Ninja**.

---

## 2. Prerequisites (install once, all on PATH)

| Tool   | How to get it                                            | Check             |
|--------|----------------------------------------------------------|-------------------|
| Git    | https://git-scm.com                                      | `git --version`   |
| Python | https://www.python.org (3.6+); tick "Add to PATH"        | `python --version`|
| CMake  | https://cmake.org/download (3.20+); "Add to PATH"        | `cmake --version` |
| Ninja  | `winget install Ninja-build.Ninja`                       | `ninja --version` |

---

## 3. Install + activate the Emscripten SDK

Pick a folder outside the project, e.g. `C:\dev`:

```bat
cd C:\dev
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
emsdk install latest
emsdk activate latest
```

Activate it in **every new terminal** (only sets PATH for the current shell):

```bat
C:\dev\emsdk\emsdk_env.bat
```

- PowerShell: run `C:\dev\emsdk\emsdk_env.ps1` instead (a `.bat`'s env changes
  do not persist back to PowerShell). If blocked: `Set-ExecutionPolicy -Scope Process Bypass`.
- Git Bash / WSL: `source /c/dev/emsdk/emsdk_env.sh`.
- To avoid re-running it: `emsdk activate latest --permanent`, or add the emsdk
  folders to your system PATH.

Verify:

```bat
emcc -v
```

> "**emcmake not found**" almost always means emsdk is not active in *this*
> shell (or you double-clicked `build.bat` from Explorer, which opens a fresh
> shell). Run `emsdk_env.bat` in the same window first.

---

## 4. Build  (run from this `web/` folder, with emsdk active)

`build.bat` checks that `emcc` / `emcmake` / `ninja` are on PATH, configures
with `emcmake cmake -G Ninja`, and builds into `.\out`.

```bat
build.bat              REM Debug build      -> .\out\Platformer.html
build.bat Release      REM Optimized build  (use this for real testing / perf)
build.bat Profile      REM Optimized + wasm function names (for the DevTools profiler)
build.bat run          REM Debug build, then serve + open with emrun
build.bat clean        REM delete .\out
```

Equivalents:
- Git Bash / WSL: `./build.sh` or `./build.sh Release`
- Raw CMake: `emcmake cmake -S . -B out -G Ninja -DCMAKE_BUILD_TYPE=Release` then `cmake --build out`
- Profile manually: add `-DAE_PROFILE=ON` to the configure line.

> **Always test with `build.bat Release`.** Debug is `-O0` + assertions and is
> far slower than your native build — it is not a fair performance comparison.
> Use `Profile` only to read a DevTools trace (it keeps function names but is a
> bit slower than Release).

First build is slow: Emscripten compiles its **SDL2 / SDL_image / FreeType**
ports once and caches them. Output in `web\out\`: `Platformer.html`, `.js`,
`.wasm`, `.data` (preloaded assets).

---

## 5. Run on this PC

You **cannot** double-click `Platformer.html` — the `.data` bundle is fetched
over HTTP, which `file://` blocks. Serve it:

```bat
emrun out\Platformer.html                              REM default browser
emrun --browser chrome out\Platformer.html             REM specific browser
emrun --list_browsers                                  REM what emrun can find
```

Or any static server: `cd out && python -m http.server 8080`, then open
`http://localhost:8080/Platformer.html`.

---

## 6. Test on iPhone / phone (no deployment)

The build is just static files, so serve from the PC and open on the phone.

### Same Wi-Fi (simplest)
```bat
emrun --hostname 0.0.0.0 --port 8080 --no_browser out\Platformer.html
ipconfig                                               REM note the IPv4 Address, e.g. 192.168.1.42
```
On the iPhone (same Wi-Fi), open `http://192.168.1.42:8080/Platformer.html`.
If it won't connect, allow `emrun`/Python through Windows Firewall (Private).

### Any network (temporary public HTTPS URL)
```bat
emrun --port 8080 --no_browser out\Platformer.html
cloudflared tunnel --url http://localhost:8080         REM prints a https://...trycloudflare.com URL (no signup)
```
(`ngrok http 8080` works too but needs a free account.)

---

## 7. Controls

| Action            | Desktop key | iPhone / touch          |
|-------------------|-------------|-------------------------|
| Move left / right | ← / →       | ◀ / ▶ buttons           |
| Jump              | Space       | JUMP button             |
| Menu (start L1/L2, quit) | 1 / 2 / Q | 1 / 2 / Q buttons   |
| Back to menu      | Esc         | MENU button             |
| Fullscreen        | F1          | ⛶ button (see §8)       |

Touch buttons appear only on touch devices. They work by dispatching synthetic
keyboard events, so they drive the exact same input path as a real keyboard
(no C++ changes).

---

## 8. Fullscreen

- **Desktop browser:** press **F1** (or the ⛶ button). Esc exits. Aspect-
  preserved (4:3 letterboxed on a widescreen monitor).
- **Android / touch tablet:** tap the **⛶** button.
- **iPhone:** iOS Safari does NOT allow fullscreen for a canvas, so F1 / ⛶ are
  no-ops. Use **Share → Add to Home Screen**, then launch from the icon — it
  runs chromeless (no Safari toolbars). Rotate to landscape for the wide view;
  the 4:3 game is letterboxed to fit (black side bars), nothing is cut off.

---

## 9. Adding another game later

Copy this `web/` folder (`CMakeLists.txt`, `shell.html`, `build.sh`,
`build.bat`) next to the new game's project, then edit `CMakeLists.txt`:
`project(<Game>_Web)`, the `AE_ROOT` relative depth if nesting differs, and the
`NAME` / `SOURCES` / `INCLUDES` / `ASSETS` paths in `ae_add_web_game()`.
Everything else (toolchain, flags, engine) is inherited from `AE/cmake/`.

---

## 10. Notes / current status

- SDL2, SDL_image and FreeType come from the **Emscripten ports** on the web —
  the native `extern/` libraries are only used by the Visual Studio build.
- **Audio is silent on web** (FMOD has no web backend; the audio API is stubbed).
- Performance: the web build runs at a smooth 60 FPS after removing the
  per-frame `glGetError()` (it is compiled out on web — it stalls WebGL badly).
- Build modes recap: **Debug** = develop, **Release** = test/ship,
  **Profile** = read a DevTools Performance trace with real function names.
