// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEFrameRateController.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2007/04/27
// Purpose			:	implementation of the frame rate controller
// History			:
// - 2007/04/27		:	- initial implementation
// - 2010/08/17 : Fixed a bug that resulted in the gAEFrameRate variable not
//                being updated. - Dan Weiss
// ---------------------------------------------------------------------------

#include "AEEngine.h"

#include <SDL.h>

// ---------------------------------------------------------------------------
// Defines





//#define FRAME_RATE_SYNC_TO_RETRACE 1

// ---------------------------------------------------------------------------
// Globals

// frame controller related variables
f64	gAEFrameRateMax;
f64	gAEFrameRate;
f64	gAEFrameTime;
f64 gAEFrameTimeMin;
u32 gAEFrameCounter;

f64	gAEFrameTimeCPU;

// ---------------------------------------------------------------------------
// Static variables

//static f64	sAEFrameTimeStart;
//static f64	sAEFrameTimeEndCPU;
//static f64	sAEFrameTimeEnd;

// Temporary
static u64 cntsPerSec = 0,
	//prevTimeStamp = 0,
	//currTimeStamp = 0,

	sTimeStampStart = 0,
	sTimeStampEnd = 0;
	/*frameCtr = 0;*/

static f64 /*frameTime = 0.0,*/
	secsPerCnt = 0.0;

// ---------------------------------------------------------------------------
// Static functions prototypes

// ---------------------------------------------------------------------------
// Functions implementations

AE_API void AEFrameRateControllerInit(u32 FrameRateMax)
{
	gAEFrameCounter	= 0;
	gAEFrameRateMax	= FrameRateMax;
	gAEFrameRate	= gAEFrameRateMax;

	if(gAEFrameRate)
	{
		gAEFrameTime	= 1.0 / gAEFrameRate;
		gAEFrameTimeMin	= 1.0 / gAEFrameRateMax;
	}
	else
	{
		gAEFrameTime	= 0.0;
		gAEFrameTimeMin	= 0.0;
	}

	
	sTimeStampStart = SDL_GetPerformanceCounter();
}

// ---------------------------------------------------------------------------

AE_API void AEFrameRateControllerReset()
{
	//AE_ASSERT_MESG(gAEFrameRateMax > 0.0, "maximum frame rate MUST be greater than 0");

	gAEFrameCounter	= 0;
	gAEFrameRate	= gAEFrameRateMax;

	if(gAEFrameRate)
	{
		gAEFrameTime	= 1.0 / gAEFrameRate;
		gAEFrameTimeMin	= 1.0 / gAEFrameRateMax;
	}
	else
	{
		gAEFrameTime	= 0.0;
		gAEFrameTimeMin	= 0.0;
	}
}



AE_API void AEFrameRateControllerStart()
{
	/*sTimeStampStart = (u64)GetSysTime();
	return;*/





	//AEGetTime(&sAEFrameTimeStart);

	cntsPerSec = SDL_GetPerformanceFrequency();
	secsPerCnt = 1.0 / static_cast<f64>(cntsPerSec);

	sTimeStampStart = SDL_GetPerformanceCounter();
	//gAEFrameTime = (sTimeStampEnd - sTimeStampStart)*secsPerCnt;
}

// ---------------------------------------------------------------------------

AE_API void AEFrameRateControllerEnd()
{

	//sTimeStampEnd = (u64)GetSysTime();
	//gAEFrameTime = (sTimeStampEnd - sTimeStampStart);

	//if (gAEFrameTime > 0.0)
	//{
	//	gAEFrameRate = (float)(1000.0 / gAEFrameTime);
	//}
	//else
	//	gAEFrameRate = 10000.0f;

	//// increment the total number of counter
	//gAEFrameCounter++;

	//return;







//#if FRAME_RATE_SYNC_TO_RETRACE == 1
//	// if the total time spent is less than the minimum required time to 
//	// maintain the maximum frame rate, wait
//	//do
//	//{
//	//	AEGetTime(&sAEFrameTimeEnd);
//	//}
//	//while((sAEFrameTimeEnd - sAEFrameTimeStart) < gAEFrameTimeMin);
//#else
//	// get the final end time
//	//AEGetTime(&sAEFrameTimeEnd);
//
//#endif
		//QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
		//secsPerCnt = 1.0 / static_cast<f64>(cntsPerSec);
#if defined(__EMSCRIPTEN__)
	// The browser paces the loop via requestAnimationFrame (emscripten_set_main_loop).
	// Do NOT busy-wait here: a spin loop inside the rAF callback overruns the frame
	// budget (causing stutter/halved FPS) and pins a CPU core at 100%.
	// Measure the REAL frame-to-frame interval so the FPS readout shows the true
	// rate (~60) instead of the inflated 1/work-time value.
	static u64 sWebPrevFrame = 0;
	sTimeStampEnd = SDL_GetPerformanceCounter();
	if (sWebPrevFrame != 0)
		gAEFrameTime = (sTimeStampEnd - sWebPrevFrame) * secsPerCnt;
	sWebPrevFrame = sTimeStampEnd;
#else
	//do
	{


		sTimeStampEnd = SDL_GetPerformanceCounter();
		gAEFrameTime = (sTimeStampEnd - sTimeStampStart)*secsPerCnt;

	}//while(gAEFrameTime < gAEFrameTimeMin);
#endif
		
	//prevTimeStamp = currTimeStamp;
	//gAEFrameTime = frameTime;

	// calculate the amount of time spend this frame
	//gAEFrameTime    = sAEFrameTimeEnd    - sAEFrameTimeStart;
	//gAEFrameTimeCPU = sAEFrameTimeEndCPU - sAEFrameTimeStart;

	//@FIXED - Reset the frame rate variable
	gAEFrameRate = 1.0 / gAEFrameTime;

	// increment the total number of counter
	gAEFrameCounter++;
}

// ---------------------------------------------------------------------------

AE_API f64 AEFrameRateControllerGetFrameTime()
{
	return gAEFrameTime;
}

// ---------------------------------------------------------------------------

AE_API u32 AEFrameRateControllerGetFrameCount()
{
	return gAEFrameCounter;
}

AE_API f64 AEFrameRateControllerGetFrameRate()
{
	return gAEFrameRate;
}

// ---------------------------------------------------------------------------
// Static functions implementations

// ---------------------------------------------------------------------------


