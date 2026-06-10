/* Start Header ************************************************************************/
/*!
\file		GameState_Platform.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file declares the main six functions of the Platform game state.
			The functions are used for both gameplay levels of the platformer game.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_GAMESTATE_PLATFORM_H_
#define CSD1130_GAMESTATE_PLATFORM_H_

#include "GameState_Platform_Extension.h"

void GameStatePlatformLoad(void);
void GameStatePlatformInit(void);
void GameStatePlatformUpdate(void);
void GameStatePlatformDraw(void);
void GameStatePlatformFree(void);
void GameStatePlatformUnload(void);

#endif // CSD1130_GAMESTATE_PLATFORM_H_