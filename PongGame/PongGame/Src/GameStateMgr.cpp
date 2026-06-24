#include "GameStateMgr.h"
#include "GameState_Pong.h"

unsigned int gGameStateInit = 0;
unsigned int gGameStateCurr = 0;
unsigned int gGameStateNext = 0;
unsigned int gGameStatePrev = 0;

void (*GameStateLoad)()   = nullptr;
void (*GameStateInit)()   = nullptr;
void (*GameStateUpdate)() = nullptr;
void (*GameStateDraw)()   = nullptr;
void (*GameStateFree)()   = nullptr;
void (*GameStateUnload)() = nullptr;

void GameStateMgrInit(unsigned int gameStateInit)
{
	gGameStateCurr = gGameStateNext = gGameStatePrev = gGameStateInit = gameStateInit;
}

void GameStateMgrUpdate()
{
	switch (gGameStateCurr)
	{
	case GS_PONG:
		GameStateLoad   = GameState_PongLoad;
		GameStateInit   = GameState_PongInit;
		GameStateUpdate = GameState_PongUpdate;
		GameStateDraw   = GameState_PongDraw;
		GameStateFree   = GameState_PongFree;
		GameStateUnload = GameState_PongUnload;
		break;
	default:
		AE_ASSERT_MESG(false, "Invalid game state.");
		break;
	}
}
