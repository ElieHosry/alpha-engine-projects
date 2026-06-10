@echo off

rem This script will:
rem - Build Alpha Engine
rem - Build Alpha Engine's documentation
rem - Consolidate tutorial files.
rem - Zip all the required files for distribution into "distrib" directory.
rem
rem The folder structure and contents of the zip file to be distributed is as follows:
rem ./
rem ./AlphaEngine/          --- contains Alpha Engine code and libs
rem ./AlphaEngine/lib/      --- Alpha Engine libs and dlls
rem ./AlphaEngine/include/  --- Alpha Engine includes
rem ./AlphaEngine/docs/     --- Alpha Engine documentation
rem ./Tutorial/             --- Alpha Engine 
rem ./Tutorial/snippets/    --- contains snippets 
rem  


set work_dir=work
set deploy_dir=deploy

set engine_dir=%work_dir%\AlphaEngine
set tutorial_dir=%work_dir%\Tutorial
set documentation_dir=%work_dir%\Documentation

set output=distrib\AlphaEngine_V3_12.zip


rem this checks if MSVC's build environment is initialized. If it is, we will jump to the "build" label.
where /q cl
IF %ERRORLEVEL% == 0 (GOTO build)
SET VC_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community
IF NOT DEFINED LIB (IF EXIST "%VC_PATH%" (call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" x64))

:build
echo [LIB] Begin Build
echo building libs
pushd build\mvs2022
devenv alpha_engine.sln /rebuild "Debug|x64"
devenv alpha_engine.sln /rebuild "Release|x64"
popd
echo [LIB] End Build

echo [PACK] Start

echo [PACK] removing %output% if it exists 
if exist %output% del /q /s %output%

echo [PACK] creating %work_dir% folder 
if exist %work_dir% rmdir /q /s %work_dir% 
mkdir %work_dir%

echo [PACK] creating %deploy_dir% folder 
if exist %deploy_dir% rmdir /q /s %deploy_dir% 
mkdir %deploy_dir%

rem Engine stuff
echo [PACK] packing libs and includes
mkdir %engine_dir% 
mkdir %engine_dir%\include
mkdir %engine_dir%\lib


xcopy include\* %engine_dir%\include\ /s /r /y /q > nul
xcopy extern\fmod\lib\fmod.dll %engine_dir%\lib\ /s /r /y /q > nul
xcopy extern\fmod\lib\fmodL.dll %engine_dir%\lib\ /s /r /y /q > nul
xcopy extern\freetype\lib\freetype.dll %engine_dir%\lib\ /s /r /y /q > nul
xcopy bin\engine\x64\Alpha_EngineD.dll %engine_dir%\lib\ /s /r /y /q > nul
xcopy bin\engine\x64\Alpha_Engine.dll %engine_dir%\lib\ /s /r /y /q > nul
xcopy bin\engine\x64\Alpha_Engine.lib %engine_dir%\lib\ /s /r /y /q > nul 
xcopy bin\engine\x64\Alpha_EngineD.lib %engine_dir%\lib\ /s /r /y /q > nul

rem Tutorial stuff
echo [TUTORIAL] START
echo [TUTORIAL] Begin Build
pushd tutorials
call build.bat
popd
echo [TUTORIAL] End Build
echo [TUTORIAL] Move to %tutorial_dir%
move tutorials\site %tutorial_dir% 

echo [TUTORIAL] Zip to %deploy_dir%
powershell Compress-Archive -Path "%tutorial_dir%" -DestinationPath "%deploy_dir%\site"
echo [TUTORIAL] END

rem Documentation stuff
echo [DOCS] START
echo [DOCS] Begin Build
pushd doc
call AEDoxygen.bat
popd
echo [DOCS] End Build


echo [DOCS] Copy to %documentation_dir%
if exist %documentation_dir% rmdir /q /s %documentation_dir% 
mkdir %documentation_dir% 
mkdir %documentation_dir%\html

xcopy doc\html\* %documentation_dir%\html\* /s /r /y /q > nul
xcopy doc\AEDocumentation.html %documentation_dir%\ /s /r /y /q > nul

echo [DOCS] Zip to %deploy_dir%
powershell Compress-Archive -Path "%documentation_dir%" -DestinationPath "%deploy_dir%\docs"
echo [DOCS] END


echo [PACK] Zipping everything...
powershell Compress-Archive -Path "%work_dir%\*" -DestinationPath "%output%"
echo [PACK] END

echo [CLEAN] Start
echo removing %work_dir% folder
rmdir /q /s %work_dir%
echo [CLEAN] End

pause
