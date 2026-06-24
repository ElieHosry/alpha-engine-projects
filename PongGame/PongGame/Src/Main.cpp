#include "Main.h"

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#endif

s8  gFontId;

// Timing globals — fixed timestep accumulator (60 Hz tick)
f64 g_fixedDT = 1.0 / 60.0;
f64 g_dt      = g_fixedDT;
f64 g_appTime = 0.0;

static void GameStartup()
{
	AESysSetWindowTitle("Pong");
	AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
	gFontId = AEGfxCreateFont("../Resources/Fonts/Arial_Italic.ttf", 24);
	GameStateMgrInit(GS_PONG);
}

static void GameShutdown()
{
	AEGfxDestroyFont(gFontId);
	AESysExit();
}

// ----------------------------------------------------------------------------
// Fixed-timestep accumulator — frame-rate-independent simulation.
// Input is sampled once per fixed step, not per rendered frame.
// ----------------------------------------------------------------------------
static f64  sPrevTime    = 0.0;
static f64  sAccumulator = 0.0;
static bool sTimerPrimed = false;

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
	if (frameTime > 0.25) frameTime = 0.25;

	sAccumulator += frameTime;

	g_dt = g_fixedDT;
	int steps = 0;
	while (sAccumulator >= g_fixedDT && steps < 5)
	{
		AEInputUpdate();

#if !defined(__EMSCRIPTEN__)
		if (AEInputCheckTriggered(AEVK_F1))
			AESysSetFullScreen(AESysIsFullScreen() ? 0 : 1);
#endif

		GameStateUpdate();
		g_appTime += g_dt;
		sAccumulator -= g_fixedDT;
		++steps;
		if (gGameStateCurr != gGameStateNext) { sAccumulator = 0.0; break; }
	}
}


#if defined(__EMSCRIPTEN__)
static bool sFresh = true;

static void AEGameTick(void)
{
	if (gGameStateCurr == GS_QUIT)
	{
		GameShutdown();
		emscripten_cancel_main_loop();
		return;
	}

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

	AESysFrameStart();
	AEUpdateFixedSteps();
	GameStateDraw();
	AESysFrameEnd();

	if (AESysDoesWindowExist() == false)
		gGameStateNext = GS_QUIT;

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
	AESysInit(nullptr, 1, 800, 600, 1, 60, false, NULL);
	GameStartup();
	emscripten_set_main_loop(AEGameTick, 0, 1);
	return 0;
}

#else
int WINAPI WinMain(HINSTANCE instanceH, HINSTANCE prevInstanceH, LPSTR command_line, int show)
{
	UNREFERENCED_PARAMETER(prevInstanceH);
	UNREFERENCED_PARAMETER(command_line);

#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	// Pin CWD to the exe's directory so ../Resources/... paths resolve correctly
	// regardless of how the process was launched (IDE, Start-Process, Explorer).
	{
		wchar_t exeDir[MAX_PATH];
		GetModuleFileNameW(NULL, exeDir, MAX_PATH);
		for (int i = (int)wcslen(exeDir) - 1; i >= 0; --i)
			if (exeDir[i] == L'\\' || exeDir[i] == L'/') { exeDir[i] = L'\0'; break; }
		SetCurrentDirectoryW(exeDir);
	}

	AESysInit(instanceH, show, 800, 600, 1, 60, false, NULL);
	GameStartup();

	while (gGameStateCurr != GS_QUIT)
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

		while (gGameStateCurr == gGameStateNext)
		{
			AESysFrameStart();
			AEUpdateFixedSteps();
			GameStateDraw();
			AESysFrameEnd();

			if (AESysDoesWindowExist() == false)
				gGameStateNext = GS_QUIT;
		}

		GameStateFree();
		if (gGameStateNext != GS_RESTART)
			GameStateUnload();

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	}

	GameShutdown();
}
#endif
