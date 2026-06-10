#!/usr/bin/env bash
# ===========================================================================
#  Build the Platformer web target (Emscripten).
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
echo "Build complete: $BUILD_DIR/Platformer.html"
echo "Run it with:    emrun \"$BUILD_DIR/Platformer.html\""
