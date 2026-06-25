# ===========================================================================
#  EmscriptenCommon.cmake
#  Shared Emscripten compile/link settings for ALL Alpha Engine web builds.
#
#  Exposes an INTERFACE target `ae_web_common` that carries the flags as usage
#  requirements, so the engine library and every game executable stay in sync.
#  Include this once per configure before defining any web target.
# ===========================================================================

if(NOT EMSCRIPTEN)
  message(FATAL_ERROR
    "Alpha Engine web targets must be configured with the Emscripten toolchain.\n"
    "Use:  emcmake cmake -S <web-dir> -B <build-dir>\n"
    "(a plain 'cmake' invocation will not work).")
endif()

# Guard against double-definition when several games are configured together.
if(TARGET ae_web_common)
  return()
endif()

add_library(ae_web_common INTERFACE)

# --- Emscripten ports (needed at BOTH compile and link time) ----------------
#  SDL2 and FreeType ship with Emscripten - the native extern/ libraries are
#  NOT used for the web build.
target_compile_options(ae_web_common INTERFACE
  "-sUSE_SDL=2"
  "-sUSE_SDL_IMAGE=2"
  "-sUSE_FREETYPE=1")

target_link_options(ae_web_common INTERFACE
  "-sUSE_SDL=2"
  "-sUSE_SDL_IMAGE=2"
  # PNG decode support for SDL_image (texture loader)
  "SHELL:-s SDL2_IMAGE_FORMATS=[\"png\"]"
  "-sUSE_FREETYPE=1"

  # --- WebGL / GLES2 (the engine's shaders are already GLES2-style) ---------
  "-sMIN_WEBGL_VERSION=1"
  "-sMAX_WEBGL_VERSION=2"
  "-sFULL_ES2=1"

  # --- Runtime --------------------------------------------------------------
  "-sALLOW_MEMORY_GROWTH=1"
  "-sEXIT_RUNTIME=0"
  "-sEXPORTED_RUNTIME_METHODS=ccall"

  # --- Debug-only aids ------------------------------------------------------
  "$<$<CONFIG:Debug>:-sASSERTIONS=1>"
  "$<$<CONFIG:Debug>:-gsource-map>")

target_compile_features(ae_web_common INTERFACE cxx_std_17)

# --- Profiling build -------------------------------------------------------
# -DAE_PROFILE=ON keeps the optimizer ON (so timings are representative) but
# preserves wasm function names, so the Chrome DevTools Performance panel shows
# real C++ function names instead of "(anonymous)". Enabled via `build.bat Profile`.
option(AE_PROFILE "Optimized build with wasm function names for profiling" OFF)
if(AE_PROFILE)
  target_compile_options(ae_web_common INTERFACE "-g2")
  target_link_options(ae_web_common INTERFACE "-g2" "--profiling-funcs")
  message(STATUS "Alpha Engine: PROFILE build (optimized + named functions).")
endif()

message(STATUS "Alpha Engine: ae_web_common configured (SDL2 + FreeType ports, WebGL).")
