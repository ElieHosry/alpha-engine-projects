/* Start Header ************************************************************************/
/*!
\file		GameStateConstants.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file declares global constant variables for the platform gameplay usage.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_GAMESTATECONSTANTS_H_
#define CSD1130_GAMESTATECONSTANTS_H_

#include "AEEngine.h"

// ----------------------------------------------------------------------------
//
// Consts
//
// ----------------------------------------------------------------------------
const unsigned int	GAME_OBJ_NUM_MAX = 32;				//The total number of different objects (Shapes)
const unsigned int	GAME_OBJ_INST_NUM_MAX = 2048;		//The total number of different game object instances

//Gameplay related variables and values
const float			GRAVITY = -20.0f;
const float			JUMP_VELOCITY = 11.0f;
const float			MOVE_VELOCITY_HERO = 4.0f;
const float			MOVE_VELOCITY_ENEMY = 7.5f;
const double		ENEMY_IDLE_TIME = 2.0;				//Enemy idle time in seconds, before changing direction
const int			HERO_LIVES = 3;
const int			GRID_WIDTH_IN_VIEWPORT = 20;		//Number of grid cells, visible in the viewport along the x-axis
const int			GRID_HEIGHT_IN_VIEWPORT = 20;		//Number of grid cells, visible in the viewport along the y-axis
const float         BOUNDING_RECT_SIZE = 1.0f;			// this is the normalized bounding rectangle (width and height) sizes - AABB collision data

//Flags
const unsigned int	FLAG_ACTIVE = 0x00000001;
const unsigned int	FLAG_VISIBLE = 0x00000002;
const unsigned int	FLAG_NON_COLLIDABLE = 0x00000004;

//Collision flags
const unsigned int	COLLISION_LEFT = 0x00000001;		//0001
const unsigned int	COLLISION_RIGHT = 0x00000002;		//0010
const unsigned int	COLLISION_TOP = 0x00000004;			//0100
const unsigned int	COLLISION_BOTTOM = 0x00000008;		//1000


#endif // CSD1130_GAMESTATECONSTANTS_H_