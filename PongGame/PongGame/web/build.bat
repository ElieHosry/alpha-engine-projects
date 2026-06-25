@echo off
REM ===========================================================================
REM  Build the Pong web target (Emscripten) on Windows.
REM
REM  Prerequisites:
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
