# ===========================================================================
#  alpha_engine/CMakeLists.txt
#  Reusable Alpha Engine library target for WEB (Emscripten) builds.
#
#  The native Windows build is UNCHANGED and still driven by
#      build/mvs2022/alpha_engine.vcxproj
#  This file is consumed only by per-game web builds via add_subdirectory();
#  Visual Studio never reads it.
#
#  ---------------------------------------------------------------------------
#  SOURCE-LEVEL ADAPTATIONS (now done - guarded by #if defined(__EMSCRIPTEN__)):
#    * include/AEEngine.h   -> AEWinShim.h instead of <windows.h> on web
#    * include/AEExport.h   -> AE_API empty on web (static link)
#    * src/AEGraphics.cpp   -> GLES2 headers + SDL_image decode (no GLEW/GDI+)
#    * src/AEAudio.cpp      -> no-op stubs on web (FMOD is native-only)
#    * src/AEInput.cpp      -> SDL keyboard/mouse on web
#    * game asset paths     -> forward slashes (work native + web MEMFS)
#  Native Windows builds are unchanged (still driven by build/mvs2022).
#  NOTE: requires the Emscripten SDL2 + SDL_image + FreeType ports (see
#  cmake/EmscriptenCommon.cmake) and is built with emcc, not plain cmake.
#  ---------------------------------------------------------------------------
# ===========================================================================

cmake_minimum_required(VERSION 3.20)
project(alpha_engine LANGUAGES C CXX)

# Pull in the shared web flags if a parent scope hasn't already (safe to
# include twice - the module self-guards).
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/EmscriptenCommon.cmake")

# Engine translation units.
file(GLOB AE_ENGINE_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(alpha_engine STATIC ${AE_ENGINE_SOURCES})

# Public headers - propagated to every game that links the engine.
target_include_directories(alpha_engine PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/src")

# SDL2 + FreeType are provided by Emscripten ports through ae_web_common.
target_link_libraries(alpha_engine PUBLIC ae_web_common)

target_compile_features(alpha_engine PUBLIC cxx_std_17)
