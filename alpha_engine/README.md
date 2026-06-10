# AlphaEngine

Digipen Singapore, Alpha Engine

Copyright (C) 2025 DigiPen Institute of Technology. Reproduction 
or disclosure of this file or its contents without the prior 
written consent of DigiPen Institute of Technology is prohibited.

## Distribution

Alpha Engine has 3 main folders to distribute:

- Development files (Include headers, static and dynamic libraries)
- Documentation via Doxygen
- Tutorial via Mkdocs

Running `distribute.bat` should build, package and zip everything nicely into the `distrib` folder.Here are the requirements you need to run `distribute.bat`:

- It must be run on Windows.
- Install Mkdocs:
  - Install Python: https://www.python.org/downloads/
  - Then install mkdocs: https://www.mkdocs.org/
  - Then install mkdocs-material: https://squidfunk.github.io/mkdocs-material/getting-started/
- Install Doxygen: https://www.doxygen.nl/manual/install.html

## History

### v3.12

- Removed `AEIsF64NearZero`. Definition was missing anyway.
- Removed `3DPipelineTools`. Not used anywhere at all.
- Removed the all the buggy shape collision functions from `AEMath`
- Removed functions that has no definitions.
- Changed documentation to use Mkdocs

### v3.11a

- Fixed bug with `AEInputMouseWheelDelta` where it does not reset back to 0.
- Fixed bug where `AEGfxColInterp` gives the wrong values.
- Fixed bug in "Multiply" blend mode.

### v3.11

- Added "Multiply" blend mode
- Added const correctness to Matrix and Vector functions
- Mouse wheel support added
- Window icon support added
- Fixed issue in font rendering; added 1px padding in generated font atlases.
- Updated tutorial with videos

### v3.10

- Graphics and Rendering changes
  - Refactored how we add colors to a mesh.
    - Added AEGfxSetColorToMultiply(). This will work like AEGfxSetTintColor(), which will set the color to be multiplied to the color of the next object to render.
    - Added AEGfxSetColorToAdd(). This will set the color to be added to the color of the next object to render.
    - Removed AEGfxSetTintColor(). 

  - Text rendering
    - AEGfxPrint() and AEGfxGetPrintSize() now correctly takes in a string as const char* instead of s8*.
    - Added transparency support for text rendering (AEGfxPrint()).

  - SetVSync() renamed to AEGfxSetVSync(). Now takes in a s32 instead of a bool.
  - AEGfxSetPosition() removed. This clashes with AEGfxSetTranform().
  - AEGfxGetPrintSize() no longer takes in references; changed to pointers.


- System Changes

  - Streamlined the minimal amount of code needed for users to get Alpha Engine running
    - AEInputUpdate() is now called in AESysFrameStart(). The documentation is also updated (it was lying that it was called in AESysFrameEnd())
    - AEAudioUpdate() is now called in AESysFrameStart().
    - AESysReset() is now called in AESysInit().

  - Improved windowed and full screen API and functionality.
    - Takes in a AESysToggleFullScreen() now takes in s32 instead of a bool, to stick to being a C API
    - AESysIsFullScreen() added.
    - AESysIsFocus() added. 
    - Fixed a bug where after toggling from fullscreen to windowed mode, the app's window remains as the highest priority window (e.g. you can't ALT-TAB away from it).


- API cleanup

  - Removed unused/confusing/inconsistent API functions that wastes users time:
    - AEGfxPoint
    - AEGfxLine
    - AEGfxTri
    - AEGfxQuad
    - AEGfxBox
    - AEGfxSphere
    - AEGfxAxis
    - AEGameStateMgr; no one could understand how to get this working. API feels confusing for both students and lecturers. Students always end up implementing their own from Game Implementation Techniques class anyway.
    - AECollision_PartX.h. It has too many conflicting structs (e.g. LineSegment) that is already provided by other parts of the API, not written in C and rarely used by students.

  - Removed ReadFromFile() from AEUtils.h; very high chance of misuse and cause of memory leaks. Students should know how to read from file themselves.

  - Matrix4.h, Point4.h and Vector4.h has been moved from the include/ folder to src/ folder (i.e. it's removed from public API). This is because they are not C-APIs, but are still used throughout the source code.

  - Replaced isZero() and isEqual() floating-point checking macros with strict type function variants: AEIsF32Zero(), AEIsF32Equal(), AEIsF64Zero(), AEIsF64Equal().


- Input changes
  - AEInputShowCursor() should now be working as intended (it was wrapping incorrectly around the 'ShowCursor' Win32 API function).


- Misc changes

  - Added VS2022 Solution.
  	- Added a sandbox project for MSVC2022 solution (named alpha_engine_sandbox). The main.cpp in that file will be a place to quickly test changes made to the alpha_engine project.
  	- Project output of binary and intemediary files for alpha_engine project is set to bin/engine/ and .tmp/engine/ respectively.
  	- Project output of binary and intemediary files for alpha_engine_sandbox project is set to bin/sandbox/ and .tmp/sandbox/ respectively.

  - Added package_library.bat that will build and zip alpha engine into the distrib folder.

  - First complete version of tutorial.

### v3.09 

- Added an audio module that uses fmod into the library. They are functions prefixed with "AEAudio".
- AESetWindowXXX functions are now correctly prefixed with AEGfxXXX. 
