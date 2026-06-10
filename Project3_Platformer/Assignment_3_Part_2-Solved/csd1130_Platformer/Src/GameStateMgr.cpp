/* Start Header ************************************************************************/
/*!
\file		GameStateMgr.cpp
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file defines all game state manager variables and implements Init and Update
			APIs

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameState_MainMenu.h"
#include "GameState_Platform.h"

// ----------------------------------------------------------------------------
//
// Globals
//
// ----------------------------------------------------------------------------

// variables to keep track the current, previous and next game state
unsigned int	gGameStateInit;
unsigned int	gGameStateCurr;
unsigned int	gGameStatePrev;
unsigned int	gGameStateNext;

// pointer to functions for game state life cycles functions
void (*GameStateLoad)()		= 0;
void (*GameStateInit)()		= 0;
void (*GameStateUpdate)()	= 0;
void (*GameStateDraw)()		= 0;
void (*GameStateFree)()		= 0;
void (*GameStateUnload)()	= 0;

// ----------------------------------------------------------------------------
//
// Call Init at the beginning
//
// ----------------------------------------------------------------------------
void GameStateMgrInit(unsigned int gameStateInit)
{
	// set the initial game state
	gGameStateInit = gameStateInit;

	// reset the current, previoud and next game
	gGameStateCurr = 
	gGameStatePrev = 
	gGameStateNext = gGameStateInit;

	// call the update to set the function pointers
	GameStateMgrUpdate();
}

// ----------------------------------------------------------------------------
//
// Update is used to set the function pointers, per game state
// We'll use the same 6 functions for both Level 1 and Level 2
//
// ----------------------------------------------------------------------------
void GameStateMgrUpdate()
{
	if ((gGameStateCurr == GS_RESTART) || (gGameStateCurr == GS_QUIT))
		return;

	switch (gGameStateCurr)
	{
	case GS_MAINMENU:
		GameStateLoad   = GameStateMainMenuLoad;
		GameStateInit   = GameStateMainMenuInit;
		GameStateUpdate = GameStateMainMenuUpdate;
		GameStateDraw   = GameStateMainMenuDraw;
		GameStateFree   = GameStateMainMenuFree;
		GameStateUnload = GameStateMainMenuUnload;
		break;
	case GS_PLATFORM1:
	case GS_PLATFORM2:
		GameStateLoad	= GameStatePlatformLoad;
		GameStateInit	= GameStatePlatformInit;
		GameStateUpdate	= GameStatePlatformUpdate;
		GameStateDraw	= GameStatePlatformDraw;
		GameStateFree	= GameStatePlatformFree;
		GameStateUnload	= GameStatePlatformUnload;
		break;
	default:
		AE_FATAL_ERROR("invalid state!!");
	}
}
