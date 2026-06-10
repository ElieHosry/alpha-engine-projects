# Place SDL2 here (native Visual Studio build only)

This folder must contain the **SDL2 development libraries for Visual C++**. The
binaries are intentionally **not committed** (they're large and third-party), so
each machine fetches them once and drops them here.

> The **web/WebAssembly build does NOT use this folder.** On the web, SDL2 comes
> from the Emscripten port (`-sUSE_SDL=2`). This folder matters only for the
> native build in `alpha_engine/build/mvs2022/`.

## What to download

Get the **SDL2 VC development package** (`SDL2-devel-2.x.x-VC.zip`) from the
official SDL releases:

- https://github.com/libsdl-org/SDL/releases  (look under the SDL2 / 2.x line,
  the file named `SDL2-devel-2.x.x-VC.zip`)
- or https://www.libsdl.org/

Unzip it and copy its contents so this folder ends up looking like the layout
below.

## Required folder layout

The Visual Studio project (`alpha_engine.vcxproj`) references these exact paths:

```
alpha_engine/extern/SDL2/
├── include/            <- all SDL2 headers (SDL.h, SDL_image.h, etc.)
│   └── SDL.h ...
└── lib/
    └── x64/            <- 64-bit libraries + runtime DLL
        ├── SDL2.lib
        ├── SDL2main.lib
        └── SDL2.dll
```

These come straight from the VC devel zip:
- `include\*`            -> `extern/SDL2/include/`
- `lib\x64\SDL2.lib`,
  `lib\x64\SDL2main.lib`,
  `lib\x64\SDL2.dll`     -> `extern/SDL2/lib/x64/`

## How the project uses it

From `alpha_engine/build/mvs2022/alpha_engine.vcxproj`:
- **Include path:** `..\..\extern\SDL2\include`
- **Library path:** `..\..\extern\SDL2\lib\x64`
- **Linked library:** `SDL2.lib`

So as long as `include/` and `lib/x64/` are populated as above, the native
build will compile and link. Make sure `SDL2.dll` ends up next to the built
`.exe` at runtime (or in `lib/x64/` so the build's copy step can grab it).

## Notes

- Use the **x64** libraries to match the project's x64 configuration.
- This project targets **SDL2** (the 2.x series) — not SDL3. Grab a 2.x release.
- The web build (`Project3_Platformer/.../web/`) ignores all of this; see
  `web/WEB_BUILD_SETUP.md`.
