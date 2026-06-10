# Alpha Engine — Web (Emscripten) Build

A scalable, per-game web build layered on top of the existing native Visual
Studio projects. **The `.vcxproj`/`.sln` files are untouched** — native builds
work exactly as before. This adds a parallel Emscripten/CMake build that turns
each game into a browser bundle (`.html` / `.js` / `.wasm` / `.data`).

## Layout

```
AE/
├── cmake/
│   ├── EmscriptenCommon.cmake   # shared flags -> INTERFACE target ae_web_common
│   └── AEWebGame.cmake          # ae_add_web_game() helper
├── alpha_engine/
│   └── CMakeLists.txt           # reusable `alpha_engine` library target (web)
└── <Game>/.../<proj>/web/
    ├── CMakeLists.txt           # ~10 lines: includes engine + calls helper
    ├── shell.html               # HTML canvas host
    └── build.sh                 # emcmake wrapper -> out/<Game>.html
```

The engine is the shared dependency; each game owns its own `web/` folder,
asset bundle, and output. Games never share a filesystem image.

## Build

```bash
# 1. activate the Emscripten SDK once per shell
source /path/to/emsdk/emsdk_env.sh

# 2. build a game
cd Project3_Platformer/Assignment_3_Part_2-Solved/csd1130_Platformer/web
./build.sh            # -> out/Platformer.html
emrun out/Platformer.html
```

## Adding a future game

1. Create a `web/` folder beside the game's native project.
2. Copy the Platformer's `CMakeLists.txt`, `shell.html`, `build.sh`.
3. In `CMakeLists.txt`, adjust only:
   - `project(<Game>_Web ...)`
   - the `AE_ROOT` relative depth if the folder nesting differs
   - the `NAME`, `SOURCES`, `INCLUDES`, and `ASSETS` paths in `ae_add_web_game()`

That's it — toolchain, flags, and the engine are inherited.

## Status / known source gaps

This is the **build architecture**. It configures correctly, but full
compilation depends on source-level platform adaptation handled in the later
porting steps (these are the same items called out in the engine
`CMakeLists.txt`):

| Source | Issue | Resolved in |
|--------|-------|-------------|
| `include/AEEngine.h` | unconditional `#include <windows.h>` | Step 3 |
| `include/AEExport.h` | `AE_API = __declspec(dllexport)` | Step 3 |
| `src/AEGraphics.cpp` | GLEW + GDI+ (desktop-only) | Step 3 / 4 |
| `src/AEAudio.cpp` | FMOD (no web backend) | Step 4+ |
| game asset paths | Windows `\` separators | Step 3 |

SDL2 and FreeType are NOT taken from `extern/` on the web — they come from the
Emscripten ports (`-sUSE_SDL=2`, `-sUSE_FREETYPE=1`).
