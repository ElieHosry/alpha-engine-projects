# ===========================================================================
#  Project3_Platformer - WEB (Emscripten) build
#  Produces Platformer.html / .js / .wasm / .data from this game's sources
#  plus the shared Alpha Engine.
#
#  Build:   ./build.sh        (requires an active emsdk environment)
#  The native Visual Studio project (csd1130_Platformer.vcxproj) is untouched.
# ===========================================================================

cmake_minimum_required(VERSION 3.20)
project(Platformer_Web LANGUAGES C CXX)

# Repo root: web/ sits 4 levels below it
#   web -> csd1130_Platformer -> Assignment_3_Part_2-Solved -> Project3_Platformer -> AE
get_filename_component(AE_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Shared build modules.
include("${AE_ROOT}/cmake/EmscriptenCommon.cmake")
include("${AE_ROOT}/cmake/AEWebGame.cmake")

# Shared engine (compiled into this build tree).
add_subdirectory("${AE_ROOT}/alpha_engine" "${CMAKE_BINARY_DIR}/alpha_engine")

# This game's translation units.
file(GLOB PLATFORMER_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/../Src/*.cpp")

ae_add_web_game(
  NAME      Platformer
  SOURCES   ${PLATFORMER_SRC}
  INCLUDES  "${CMAKE_CURRENT_SOURCE_DIR}/../Include"
  ASSETS    "${CMAKE_CURRENT_SOURCE_DIR}/../../Resources"
  SHELL     "${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
)
