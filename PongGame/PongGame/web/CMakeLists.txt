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
