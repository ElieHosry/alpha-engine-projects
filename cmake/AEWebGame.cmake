# ===========================================================================
#  AEWebGame.cmake
#  ae_add_web_game(): the one-call helper each game uses to produce a browser
#  build (.html / .js / .wasm / .data) from its own sources + the shared engine.
#
#  Usage (from a game's web/CMakeLists.txt):
#
#      ae_add_web_game(
#        NAME        Platformer
#        SOURCES     ${GAME_SRC}          # list of .cpp files
#        INCLUDES    ../Include           # extra include dirs (optional)
#        ASSETS      ../../Resources      # folder to preload (optional)
#        ASSET_MOUNT Resources            # virtual mount point (default: Resources)
#        SHELL       shell.html           # custom HTML template (optional)
#      )
#
#  Adding a new game is then ~5 lines - the engine, toolchain, and flags are
#  all inherited from the shared cmake/ modules.
# ===========================================================================

include_guard(GLOBAL)

function(ae_add_web_game)
  set(options)
  set(oneValueArgs   NAME ASSETS ASSET_MOUNT SHELL)
  set(multiValueArgs SOURCES INCLUDES)
  cmake_parse_arguments(G "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT G_NAME)
    message(FATAL_ERROR "ae_add_web_game: NAME is required")
  endif()
  if(NOT G_SOURCES)
    message(FATAL_ERROR "ae_add_web_game(${G_NAME}): SOURCES is required")
  endif()
  if(NOT TARGET alpha_engine)
    message(FATAL_ERROR
      "ae_add_web_game(${G_NAME}): the 'alpha_engine' target is missing.\n"
      "add_subdirectory(<AE_ROOT>/alpha_engine ...) before calling this.")
  endif()

  add_executable(${G_NAME} ${G_SOURCES})

  # Shared engine + shared web flags.
  target_link_libraries(${G_NAME} PRIVATE alpha_engine ae_web_common)

  if(G_INCLUDES)
    target_include_directories(${G_NAME} PRIVATE ${G_INCLUDES})
  endif()

  # Emit <NAME>.html (Emscripten also produces the matching .js/.wasm).
  set_target_properties(${G_NAME} PROPERTIES SUFFIX ".html")

  # Optional custom HTML shell.
  if(G_SHELL)
    target_link_options(${G_NAME} PRIVATE "--shell-file=${G_SHELL}")
    set_property(TARGET ${G_NAME} APPEND PROPERTY LINK_DEPENDS "${G_SHELL}")
  endif()

  # Optional per-game asset bundle -> preloaded into the virtual filesystem.
  # Each game gets its OWN .data image (games never share a filesystem).
  if(G_ASSETS)
    if(NOT G_ASSET_MOUNT)
      set(G_ASSET_MOUNT "Resources")
    endif()
    target_link_options(${G_NAME} PRIVATE
      "--preload-file" "${G_ASSETS}@${G_ASSET_MOUNT}")
    set_property(TARGET ${G_NAME} APPEND PROPERTY LINK_DEPENDS "${G_ASSETS}")
    message(STATUS "  ${G_NAME}: preloading '${G_ASSETS}' -> /${G_ASSET_MOUNT}")
  endif()

  message(STATUS "Alpha Engine web game: ${G_NAME} -> ${G_NAME}.html")
endfunction()
