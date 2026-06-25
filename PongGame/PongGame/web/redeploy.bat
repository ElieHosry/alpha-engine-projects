@echo off
REM ===========================================================================
REM  redeploy.bat - one-command publish to GitHub Pages (pong-web).
REM
REM  Does: Release build -> copy out\ into your Pages repo (Pong.html
REM        becomes index.html, ensures .nojekyll) -> git add/commit/push.
REM
REM  ONE-TIME SETUP: set REPO_DIR below to your local clone of the pong-web
REM  repo (the folder you ran `git clone https://github.com/ElieHosry/pong-web`).
REM
REM  Usage:
REM    redeploy.bat                       build + deploy (auto commit message)
REM    redeploy.bat "fixed paddle bug"    build + deploy with a commit message
REM    redeploy.bat nobuild               skip the build, just redeploy out\
REM ===========================================================================
setlocal EnableExtensions

REM ---- EDIT THIS to your pong-web Pages repo's local folder -----------------
set "REPO_DIR=C:\Users\ehosry_local\Desktop\FullStack02\AE\PongGame\PongGame\web\out-files"
REM ---------------------------------------------------------------------------

set "HERE=%~dp0"
set "OUT=%HERE%out"
set "DO_BUILD=1"
set "MSG=%~1"

if /I "%~1"=="nobuild" ( set "DO_BUILD=0" & set "MSG=%~2" )
if "%MSG%"=="" set "MSG=update web build"

REM ---- sanity checks ---------------------------------------------------------
if not exist "%REPO_DIR%\" (
    echo ERROR: REPO_DIR not found: "%REPO_DIR%"
    echo Edit redeploy.bat and set REPO_DIR to your local pong-web repo folder.
    exit /b 1
)
if not exist "%REPO_DIR%\.git\" (
    echo ERROR: "%REPO_DIR%" is not a git repository ^(no .git folder^).
    exit /b 1
)
where git >nul 2>nul || ( echo ERROR: git not found on PATH. & exit /b 1 )

REM ---- build (Release) -------------------------------------------------------
if "%DO_BUILD%"=="1" (
    echo === Building Release ===
    call "%HERE%build.bat" Release
    if errorlevel 1 ( echo Build FAILED - aborting deploy. & exit /b 1 )
)

if not exist "%OUT%\Pong.html" (
    echo ERROR: "%OUT%\Pong.html" not found. Build first ^(or run without nobuild^).
    exit /b 1
)

REM ---- copy build output into the repo (Pong.html -> index.html) ------------
echo === Copying build output to "%REPO_DIR%" ===
copy /Y "%OUT%\Pong.html" "%REPO_DIR%\index.html"  >nul || goto :copyfail
copy /Y "%OUT%\Pong.js"   "%REPO_DIR%\Pong.js"     >nul || goto :copyfail
copy /Y "%OUT%\Pong.wasm" "%REPO_DIR%\Pong.wasm"   >nul || goto :copyfail
if exist "%OUT%\Pong.data" copy /Y "%OUT%\Pong.data" "%REPO_DIR%\Pong.data" >nul
if not exist "%REPO_DIR%\.nojekyll" type nul > "%REPO_DIR%\.nojekyll"

REM ---- commit + push ---------------------------------------------------------
pushd "%REPO_DIR%"
git add -A
git diff --cached --quiet
if errorlevel 1 (
    echo === Committing + pushing ===
    git commit -m "%MSG%"  || ( popd & echo Commit FAILED. & exit /b 1 )
    git push               || ( popd & echo Push FAILED ^(check auth / run 'git pull' first^). & exit /b 1 )
    echo.
    echo Deployed. GitHub Pages will update in ~1 minute.
    echo Live URL: https://ElieHosry.github.io/pong-web/
) else (
    echo Nothing changed since last deploy - skipping commit/push.
)
popd

endlocal
exit /b 0

:copyfail
echo ERROR: failed to copy build files to "%REPO_DIR%".
exit /b 1
