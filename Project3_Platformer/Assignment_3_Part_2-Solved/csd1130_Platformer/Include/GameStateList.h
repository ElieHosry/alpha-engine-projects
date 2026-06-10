/* Start Header ************************************************************************/
/*!
\file		GameStateList.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file list an enumeration of possible game states

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_GAMESTATELIST_H_
#define CSD1130_GAMESTATELIST_H_

// ----------------------------------------------------------------------------
//
// enumeration for all possible different game states
//
// ----------------------------------------------------------------------------
enum
{
	// list of all game states 
	GS_PLATFORM1 = 0,
	GS_PLATFORM2,
	GS_MAINMENU,

	// special game state IDs
	GS_RESTART,
	GS_QUIT,
	GS_NUM
};

#endif // CSD1130_GAMESTATELIST_H_