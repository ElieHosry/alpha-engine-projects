// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AESystem.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/01/31
// Purpose			:	implementation of the system components
// History			:
// - 2008/01/31		:	- initial implementation
// ---------------------------------------------------------------------------

#include "AEEngine.h"

#include <SDL.h>
#if !defined(__EMSCRIPTEN__)
#include <SDL_syswm.h>		// only needed to fetch the native Win32 HWND
#else
#include <emscripten/html5.h>	// browser fullscreen API
#endif

// ---------------------------------------------------------------------------
// global variables
#define		AE_VERSION				3.12

// window related variables
HINSTANCE	ghAESysAppInstance;
HWND		gAESysWindowHandle;
SDL_Window*	gAESDLWindow = nullptr;		// SDL owns the OS window + GL surface

const char*	gpAESysWinTitle		= "Alpha Engine";
const char*	gpAESysWinClassName	= "Window Class";

//AE_API s32	gAESysAppActive;

s32	gAESysWinExists;

s32 gAEIsFullScreen = 0;
s32 gAEIsFocus = 1;
f32 gResolutionRatioX = 1.0f;
f32 gResolutionRatioY = 1.0f;

// ---------------------------------------------------------------------------
// Externs 
extern s32 mMouseWheelDeltaPrev;

// ---------------------------------------------------------------------------
// Static variables

// ---------------------------------------------------------------------------
// Static function prototypes

// ---------------------------------------------------------------------------

// Retained only to satisfy a legacy extern reference in AEGraphics.cpp.
// No longer registered with the OS - SDL now owns the window.
// WNDCLASS does not exist on the web, so this is native-only.
#if !defined(__EMSCRIPTEN__)
WNDCLASS	winClass;
#endif

#if defined(__EMSCRIPTEN__)
// --- Web fullscreen (browser HTML5 API) ------------------------------------
// Fullscreen on the web MUST be requested from inside a real user-input event;
// the requestAnimationFrame game loop does not count as a gesture. So F1 is
// handled here via an HTML5 keydown callback, which calls AESysSetFullScreen
// from within the gesture. AESysSetFullScreen() uses an aspect-preserving scale
// strategy that keeps the 800x600 backing buffer and letterboxes to fill the
// screen, so the engine's fixed glViewport needs no changes.
static EM_BOOL AE_OnFullscreenChange(int /*type*/, const EmscriptenFullscreenChangeEvent* e, void* /*ud*/)
{
	gAEIsFullScreen = e->isFullscreen ? 1 : 0;	// keep engine state in sync (incl. Esc-to-exit)
	return EM_TRUE;
}

static EM_BOOL AE_OnKeyDown(int /*type*/, const EmscriptenKeyboardEvent* e, void* /*ud*/)
{
	if (e->keyCode == 112 /* F1 */ && !e->repeat)
	{
		AESysSetFullScreen(gAEIsFullScreen ? 0 : 1);	// inside the user gesture -> allowed
		return EM_TRUE;									// consume (suppress browser Help)
	}
	return EM_FALSE;
}
#endif

AE_API s32 AESysInit(
	HINSTANCE hAppInstance,
	s32 show, 
	s32 WinWidth, 
	s32 WinHeight, 
	s32 CreateConsole, 
	u32 FrameRateMax, 
	bool vsync, 
	LRESULT(CALLBACK* pWinCallBack)(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp))
{
#if !defined(__EMSCRIPTEN__)
	// For cater for inconsistent screen resolution issues between different monitor DPI.
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
#endif

	// keep a copy of the handle to the application
	ghAESysAppInstance = hAppInstance;

#if !defined(__EMSCRIPTEN__)
	// Win32 console allocation. On the web, printf already goes to the browser console.
	if (CreateConsole)
	{
		if (AllocConsole())
		{
			FILE* file;

			freopen_s(&file, "CONOUT$", "wt", stdout);
			freopen_s(&file, "CONOUT$", "wt", stderr);
			freopen_s(&file, "CONOUT$", "wt", stdin);

			SetConsoleTitle(TEXT("Alpha Engine - Console"));
		}
	}
#else
	(void)CreateConsole;
#endif

	printf("Alpha Engine version: %2.2f\n\n", AE_VERSION);

	// pWinCallBack is ignored on the SDL backend (kept for API compatibility).
	(void)pWinCallBack;

	// We keep WinMain as the entry point, so tell SDL the main function is
	// ready before initializing (we are NOT going through SDL_main).
	SDL_SetMainReady();
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
	{
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return 0;
	}

	// Request a double-buffered OpenGL surface with depth + stencil.
	// (GLES2-compatible feature set; mirrors the old PIXELFORMATDESCRIPTOR.)
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

	Uint32 winFlags = SDL_WINDOW_OPENGL;
	if (show == 0)						// SW_HIDE
		winFlags |= SDL_WINDOW_HIDDEN;

	gAESDLWindow = SDL_CreateWindow(
		gpAESysWinTitle,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		WinWidth, WinHeight, winFlags);

	if (gAESDLWindow == NULL)
	{
		printf("SDL_CreateWindow failed: %s\n", SDL_GetError());
		return 0;
	}

	// Retrieve the native Win32 HWND so the rest of the engine (window icon,
	// cursor mapping in AEInput, GDI+ texture loader) keeps working unchanged.
#if !defined(__EMSCRIPTEN__)
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	if (SDL_GetWindowWMInfo(gAESDLWindow, &wmInfo))
		gAESysWindowHandle = wmInfo.info.win.window;
#else
	gAESysWindowHandle = nullptr;	// no native window handle in the browser
#endif

	// window currently exists
	gAESysWinExists = 1;

#if defined(__EMSCRIPTEN__)
	// F1 -> fullscreen toggle, routed through a real keydown event (gesture),
	// plus a change listener so Esc-to-exit keeps gAEIsFullScreen accurate.
	emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, AE_OnKeyDown);
	emscripten_set_fullscreenchange_callback(EMSCRIPTEN_EVENT_TARGET_DOCUMENT, nullptr, EM_TRUE, AE_OnFullscreenChange);
#endif

	// initialize the graphics
	if (AEGfxInit(WinWidth, WinHeight) == 0)
		return 0;

	// initialize the input
	if (AEInputInit() == 0) {
		printf("Audio module failed to initialize");
		return 0;
	}

	// initialize the input
	if (AEAudioInit() == 0) {
		printf("Audio module failed to initialize");
		return 0;
	}

	// initialize the frame controller
	//if (AEFrameRateControllerInit() == 0)
	//return 0;
	AEFrameRateControllerInit(FrameRateMax);

	// do the initial reset
	AESysReset();

	AEGfxSetVSync(vsync);

	return 1;

}


// ---------------------------------------------------------------------------

AE_API void AESysReset(void)
{

	// reset the frame controller
	AEFrameRateControllerReset();
	AEInputReset();

}

// ---------------------------------------------------------------------------

AE_API void AESysUpdate(void)
{
	SDL_Event e;

	// pump the SDL event queue (replaces the Win32 PeekMessage loop)
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{
		case SDL_QUIT:
			gAESysWinExists = 0;
			break;

		case SDL_MOUSEWHEEL:
			// feed the wheel delta to AEInput (was WM_MOUSEWHEEL in the old WndProc)
			mMouseWheelDeltaPrev = e.wheel.y;
			break;

		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
				gAEIsFocus = 1;
			else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
				gAEIsFocus = 0;
			else if (e.window.event == SDL_WINDOWEVENT_CLOSE)
				gAESysWinExists = 0;
			break;
		}
	}
}

// ---------------------------------------------------------------------------

AE_API void AESysExit(void)
{
#if !defined(__EMSCRIPTEN__)
	//New line CODE
	FreeConsole();
#endif

	AEAudioExit();

	AEInputExit();

	AEGfxExit();	// destroys the GL context (still owned by the SDL window)

	if (gAESDLWindow)
	{
		SDL_DestroyWindow(gAESDLWindow);
		gAESDLWindow = nullptr;
	}
	SDL_Quit();
}

// ---------------------------------------------------------------------------

AE_API void AESysFrameStart()
{	
	// tell the frame rate controller that a new frame is starting
	AEFrameRateControllerStart();	   


	// tell graphics module that a new frame is starting
	AEGfxStart();

	// NOTE: input is now sampled once per FIXED SIMULATION STEP by the game loop
	// (AEUpdateFixedSteps), not once per rendered frame. With the fixed-timestep
	// accumulator the update can be skipped on some frames; sampling input here
	// (every frame) would advance the prev/curr key state and erase one-frame
	// "triggered" edges before GameStateUpdate could read them, dropping key
	// presses (e.g. menu 1/2/Q). Sampling per step keeps input and updates in
	// lock-step so no edge is lost.
	//AEInputUpdate();

	// Handling Audio
	AEAudioUpdate();
}

// ---------------------------------------------------------------------------

AE_API void AESysFrameEnd()
{
	// tell the graphics module that the current frame is finish
	AEGfxEnd();
	
	
	// handle windows message
	// * do it outside the frame start => frame end so that it will not effect 
	//   the game loop timing.
	AESysUpdate();

	// tell the frame rate controller that the frame is done
	AEFrameRateControllerEnd();
}

// ---------------------------------------------------------------------------

AE_API HWND AESysGetWindowHandle()
{
	return gAESysWindowHandle;
}

// ---------------------------------------------------------------------------
/*
AE_API s32* AESysGetAppActive()
{
	return &gAESysAppActive;
}
*/
// ---------------------------------------------------------------------------

AE_API void AESysSetWindowTitle(const char* pTitle)
{
	SDL_SetWindowTitle(gAESDLWindow, pTitle);
}

// ---------------------------------------------------------------------------

AE_API s32 AESysDoesWindowExist()
{
	return gAESysWinExists;
}

// ---------------------------------------------------------------------------

AE_API s32 AESysIsFullScreen() {
	return gAEIsFullScreen;
}

AE_API s32 AESysIsFocus() {
	return gAEIsFocus;
}

AE_API void AESysSetWindowIcon(const char* filename, s32 width, s32 height)
{
#if !defined(__EMSCRIPTEN__)
	HICON icon = (HICON)LoadImageA(NULL, filename, IMAGE_ICON, width, height, LR_LOADFROMFILE);
	// Set the large icon (for Alt+Tab dialog)
	SendMessage(gAESysWindowHandle, WM_SETICON, ICON_BIG, (LPARAM)icon);

	// Set the small icon (for window's title bar)
	SendMessage(gAESysWindowHandle, WM_SETICON, ICON_SMALL, (LPARAM)icon);
#else
	// Web: the browser tab favicon is set in the HTML shell; no-op here.
	(void)filename; (void)width; (void)height;
#endif
}


AE_API void AESysSetFullScreen(s32 fullScreen)
{
	gAEIsFullScreen = !!fullScreen;

#if defined(__EMSCRIPTEN__)
	if (gAEIsFullScreen)
	{
		// Aspect-preserving fullscreen: keep the 800x600 backing buffer and let
		// the browser scale the canvas to fill the screen (letterboxed). This
		// avoids touching the engine's fixed glViewport.
		EmscriptenFullscreenStrategy strat;
		memset(&strat, 0, sizeof(strat));
		strat.scaleMode                 = EMSCRIPTEN_FULLSCREEN_SCALE_ASPECT;
		strat.canvasResolutionScaleMode = EMSCRIPTEN_FULLSCREEN_CANVAS_SCALE_NONE;
		strat.filteringMode             = EMSCRIPTEN_FULLSCREEN_FILTERING_DEFAULT;
		emscripten_request_fullscreen_strategy("#canvas", EM_TRUE, &strat);
	}
	else
	{
		emscripten_exit_fullscreen();
	}
#else
	if (AESysIsFullScreen())
	{   // enables full screen (desktop / borderless)
		// keep the cursor-coordinate scale factor used by AEInput in sync
		SDL_DisplayMode dm;
		if (SDL_GetDesktopDisplayMode(SDL_GetWindowDisplayIndex(gAESDLWindow), &dm) == 0)
		{
			gResolutionRatioX = (f32)dm.w / (f32)AEGfxGetWindowWidth();
			gResolutionRatioY = (f32)dm.h / (f32)AEGfxGetWindowHeight();
		}

		SDL_SetWindowFullscreen(gAESDLWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
	}
	else
	{   // disables full screen
		gResolutionRatioX = 1.0f;
		gResolutionRatioY = 1.0f;

		SDL_SetWindowFullscreen(gAESDLWindow, 0);
		SDL_SetWindowSize(gAESDLWindow, AEGfxGetWindowWidth(), AEGfxGetWindowHeight());
		SDL_SetWindowPosition(gAESDLWindow, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
	}
#endif
}

// ---------------------------------------------------------------------------

