/* Start Header ************************************************************************/
/*!
\file		Main.cpp
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		Entry point file with WinMain implementing the 2 nested loops.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Main.h"
#include <memory>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

// ----------------------------------------------------------------------------
//
// Globals
//
// ----------------------------------------------------------------------------
s8		fontId;			// Font for drawing text


// ----------------------------------------------------------------------------
//
// Shared startup used by both the native and web entry points.
// Forward-slash asset paths work on Windows AND in the Emscripten virtual FS
// (assets are preloaded under /Resources; "../Resources" resolves to it).
//
// ----------------------------------------------------------------------------
static void GameStartup()
{
	// Changing the window title
	AESysSetWindowTitle("Platformer Demo!");

	// Set background color
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);

	// Font is created here, and is used for all your levels
	fontId = AEGfxCreateFont("../Resources/Fonts/Arial_Italic.ttf", 24);

	GameStateMgrInit(GS_MAINMENU);
}

static void GameShutdown()
{
	//free you font here
	AEGfxDestroyFont(fontId);

	// free the system
	AESysExit();
}

// ----------------------------------------------------------------------------
//
// Fixed-timestep accumulator. Advances the simulation in fixed g_fixedDT ticks
// based on REAL elapsed wall-clock time, so object speed is identical at any
// frame rate (web ~60Hz via requestAnimationFrame, native uncapped, 120Hz
// monitors, slow phones, ...). The frame is still RENDERED once per call by the
// caller. This replaces the old "g_dt = g_fixedDT once per frame" which tied
// motion speed to the frame rate.
//
// ----------------------------------------------------------------------------
static f64  sPrevTime    = 0.0;
static f64  sAccumulator = 0.0;
static bool sTimerPrimed = false;

// Call when (re)entering a game state so load time is not counted as elapsed.
static void AEResetFrameTimer()
{
	sTimerPrimed = false;
	sAccumulator = 0.0;
}

static void AEUpdateFixedSteps()
{
	const f64 now = AEGetTime(nullptr);
	if (!sTimerPrimed) { sPrevTime = now; sTimerPrimed = true; }

	f64 frameTime = now - sPrevTime;
	sPrevTime = now;
	if (frameTime > 0.25) frameTime = 0.25;          // clamp big hitches (tab switch / load)

	sAccumulator += frameTime;

	g_dt = g_fixedDT;                                 // one simulation tick = one fixed step
	int steps = 0;
	while (sAccumulator >= g_fixedDT && steps < 5)    // cap steps to avoid spiral of death
	{
		// Sample input once PER STEP (not per render frame) so one-frame
		// "triggered" edges line up 1:1 with the update that reads them.
		AEInputUpdate();

#if !defined(__EMSCRIPTEN__)
		// Native F1 fullscreen toggle (web handles F1 via an HTML5 keydown
		// callback). Checked here so it is consumed exactly once per key edge.
		if (AEInputCheckTriggered(AEVK_F1))
			AESysSetFullScreen(AESysIsFullScreen() ? 0 : 1);
#endif

		GameStateUpdate();
		g_appTime += g_dt;
		sAccumulator -= g_fixedDT;
		++steps;
		if (gGameStateCurr != gGameStateNext) { sAccumulator = 0.0; break; }  // state change requested
	}
}


#if defined(__EMSCRIPTEN__)
// ----------------------------------------------------------------------------
//
// Web entry point: the browser owns the loop, so the original nested while()
// loops are flattened into a single per-frame tick driven by
// emscripten_set_main_loop. sFresh tracks the "load/init a new state" phase
// that the outer loop used to perform.
//
// ----------------------------------------------------------------------------
static bool sFresh = true;

static void AEGameTick(void)
{
	// Whole program done -> tear down and stop the browser loop.
	if (gGameStateCurr == GS_QUIT)
	{
		GameShutdown();
		emscripten_cancel_main_loop();
		return;
	}

	// Enter a (new) game state: reset + load + init, once.
	if (sFresh)
	{
		AESysReset();

		if (gGameStateCurr != GS_RESTART)
		{
			GameStateMgrUpdate();
			GameStateLoad();
		}
		else
			gGameStateNext = gGameStateCurr = gGameStatePrev;

		GameStateInit();
		AEResetFrameTimer();
		sFresh = false;
	}

	// One rendered frame; the simulation advances by REAL elapsed time in fixed
	// steps (see AEUpdateFixedSteps), so speed is frame-rate independent.
	AESysFrameStart();
	AEUpdateFixedSteps();
	GameStateDraw();
	AESysFrameEnd();

	if (AESysDoesWindowExist() == false)
		gGameStateNext = GS_QUIT;

	// State transition: free/unload, then flag the next state to load.
	if (gGameStateCurr != gGameStateNext)
	{
		GameStateFree();

		if (gGameStateNext != GS_RESTART)
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
		sFresh = true;
	}
}

int main(int /*argc*/, char* /*argv*/[])
{
	// HINSTANCE/show are unused on the SDL/web backend.
	AESysInit(nullptr, 1, 800, 600, 1, 60, false, NULL);

	GameStartup();

	// fps = 0 -> use requestAnimationFrame; simulate_infinite_loop = 1.
	emscripten_set_main_loop(AEGameTick, 0, 1);
	return 0;
}

#else
// ----------------------------------------------------------------------------
//
// Native entry point (unchanged behaviour): the classic blocking nested loops.
//
// ----------------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	UNREFERENCED_PARAMETER(prevInstanceH);
	UNREFERENCED_PARAMETER(command_line);

	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Initialize the system
	AESysInit (instanceH, show, 800, 600, 1, 60, false, NULL);

	GameStartup();

	while (gGameStateCurr != GS_QUIT)
	{
		// Reset the system modules
		AESysReset();

		// If not restarting, load the gamestate
		if (gGameStateCurr != GS_RESTART)
		{
			GameStateMgrUpdate();
			GameStateLoad();
		}
		else
			gGameStateNext = gGameStateCurr = gGameStatePrev;

		// Initialize the gamestate
		GameStateInit();
		AEResetFrameTimer();

		while (gGameStateCurr == gGameStateNext)
		{
			AESysFrameStart();

			// Simulation advances by REAL elapsed time in fixed steps, so motion
			// speed no longer depends on the frame rate (matches the web build).
			AEUpdateFixedSteps();

			GameStateDraw();

			AESysFrameEnd();

			// check if forcing the application to quit
			if (AESysDoesWindowExist() == false)
				gGameStateNext = GS_QUIT;
			// (F1 fullscreen toggle now handled per fixed step in AEUpdateFixedSteps)
		}

		GameStateFree();

		if(gGameStateNext != GS_RESTART)
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	}

	GameShutdown();
}
#endif