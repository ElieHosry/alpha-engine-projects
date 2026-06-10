/* Start Header ************************************************************************/
/*!
\file		GameStateMgr.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file declares all game state manager variables along with Init and Update
			APIs

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_GAMESTATEMGR_H_
#define CSD1130_GAMESTATEMGR_H_

#include "AEEngine.h"
#include "GameStateList.h"

// ----------------------------------------------------------------------------
//
// Externs
//
// ----------------------------------------------------------------------------
extern unsigned int gGameStateInit;
extern unsigned int gGameStateCurr;
extern unsigned int gGameStatePrev;
extern unsigned int gGameStateNext;

extern void (*GameStateLoad)();
extern void (*GameStateInit)();
extern void (*GameStateUpdate)();
extern void (*GameStateDraw)();
extern void (*GameStateFree)();
extern void (*GameStateUnload)();


void GameStateMgrInit(unsigned int gameStateInit);

void GameStateMgrUpdate();

#endif // CSD1130_GAMESTATEMGR_H_