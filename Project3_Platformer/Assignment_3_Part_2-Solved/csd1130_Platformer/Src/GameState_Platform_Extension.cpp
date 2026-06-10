/* Start Header ************************************************************************/
/*!
\file		GameState_Platform_Extension.cpp
\author
\par
\date
\brief

Copyright (C) 20xx DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameState_Platform_Extension.h"

#include <iostream>
#include <fstream>


// =========================================================
// 
// additional new variables - deprecated
// 
// =========================================================
bool			facingRight = false;
f64				wallJumpLeft = 0.0;
f64				wallJumpRight = 0.0;
bool			youWon = false;


// =========================================================
// 
// import map data for the current level (level 1 or level 2)
// 
// =========================================================
void Import_MapData(void)
{
	//Setting intital binary map values
	MapData = nullptr;
	BinaryCollisionArray = nullptr;
	BINARY_MAP_WIDTH = 0;
	BINARY_MAP_HEIGHT = 0;

	//Importing Data - Must Quit the application if "ImportMapDataFromFile" fails
	if (gGameStateCurr == GS_PLATFORM1)
	{
		if (!ImportMapDataFromFile("../Resources/Levels/Exported.txt"))
			gGameStateNext = GS_QUIT;
	}
	else if (gGameStateCurr == GS_PLATFORM2)
	{
		if (!ImportMapDataFromFile("../Resources/Levels/Exported2.txt"))
			gGameStateNext = GS_QUIT;
	}
}

// =========================================================
// 
// compute MapTransform matrix - Only once at load time
// 
// =========================================================
void Compute_MapTransformMatrix(void)
{
	//Computing the matrix which take a point out of the normalized coordinates system
	//of the binary map
	/***********
	Compute a transformation matrix and save it in "MapTransform".
	This transformation transforms any point from the normalized coordinates system of the binary map.
	Later on, when rendering each object instance, we should concatenate "MapTransform" with the
	object instance's own transformation matrix

	Compute a translation matrix (-GRID_WIDTH_IN_VIEWPORT/2, -GRID_WIDTH_IN_VIEWPORT/2) and save it in "trans"
	Compute a scaling matrix and save it in "scale". 
		The scale must account for the window width and height.
		Alpha engine has 2 helper functions to get the window width and height: AEGfxGetWindowWidth() and AEGfxGetWindowHeight()
		Example: scale along x-axis = AEGfxGetWindowWidth() / static_cast<f32>(GRID_WIDTH_IN_VIEWPORT)
	Concatenate scale and translate and save the result in "MapTransform"
	***********/
	AEMtx33 scale, trans;

	AEMtx33Trans(&trans, -GRID_WIDTH_IN_VIEWPORT * 0.5f, -GRID_HEIGHT_IN_VIEWPORT * 0.5f);
	AEMtx33Scale(&scale, AEGfxGetWindowWidth() / static_cast<f32>(GRID_WIDTH_IN_VIEWPORT), AEGfxGetWindowHeight() / static_cast<f32>(GRID_HEIGHT_IN_VIEWPORT));
	AEMtx33Concat(&MapTransform, &scale, &trans);
}

// =========================================================
// 
// creates all initial game objects instances
// 
// =========================================================
void Starting_GameObjectsInstances(void)
{
	int i = 0, j = 0;

	pHero = nullptr;
	pBlackInstance = nullptr;
	pWhiteInstance = nullptr;
	TotalCoins = 0;

	//Create an object instance representing the black cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a non collision cell, we position this instance in the correct location and then we render it
	AEVec2 scl = { 1.0f, 1.0f };
	pBlackInstance = gameObjInstCreate(TYPE_OBJECT_EMPTY, &scl, 0, 0, 0.0f);
	pBlackInstance->flag ^= FLAG_VISIBLE;
	pBlackInstance->flag |= FLAG_NON_COLLIDABLE;

	//Create an object instance representing the white cell.
	//This object instance should not be visible. When rendering the grid cells, each time we have
	//a collision cell, we position this instance in the correct location and then we render it
	pWhiteInstance = gameObjInstCreate(TYPE_OBJECT_COLLISION, &scl, 0, 0, 0.0f);
	pWhiteInstance->flag ^= FLAG_VISIBLE;
	pWhiteInstance->flag |= FLAG_NON_COLLIDABLE;

	//Setting the inital number of hero lives
	HeroLives = HERO_LIVES;

	GameObjInst* pInst;
	AEVec2 Pos;

	// creating the main character, the enemies and the coins according 
	// to their initial positions in MapData

	/***********
	Loop through all the array elements of MapData
	(which was initialized in the "GameStatePlatformLoad" function
	from the .txt file)
		if the element represents a collidable or non collidable area
			don't do anything

		if the element represents the hero
			Create a hero instance
			Set its position depending on its array indices in MapData
			Save its array indices in Hero_Initial_X and Hero_Initial_Y
			(Used when the hero dies and its position needs to be reset)

		if the element represents an enemy
			Create an enemy instance
			Set its position depending on its array indices in MapData

		if the element represents a coin
			Create a coin instance
			Set its position depending on its array indices in MapData

	***********/
	for (i = 0; i < BINARY_MAP_WIDTH; ++i)
		for (j = 0; j < BINARY_MAP_HEIGHT; ++j)
		{
			if (MapData[j][i] == TYPE_OBJECT_HERO)
			{
				Pos.x = static_cast<f32>(i) + 0.5f;
				Pos.y = static_cast<f32>(j) + 0.5f;
				AEVec2 scale{ 1.0f, 1.0f };
				pHero = gameObjInstCreate(TYPE_OBJECT_HERO, &scale, &Pos, 0, 0.0f);
				Hero_Initial_X = i;
				Hero_Initial_Y = j;
			}
			if (MapData[j][i] == TYPE_OBJECT_ENEMY)
			{
				Pos.x = static_cast<f32>(i) + 0.5f;
				Pos.y = static_cast<f32>(j) + 0.5f;
				AEVec2 scale{ 1.0f, 1.0f };
				pInst = gameObjInstCreate(TYPE_OBJECT_ENEMY, &scale, &Pos, 0, 0.0f);
			}
			if (MapData[j][i] == TYPE_OBJECT_COIN)
			{
				Pos.x = static_cast<f32>(i) + 0.5f;
				Pos.y = static_cast<f32>(j) + 0.5f;
				AEVec2 scale{ 1.0f, 1.0f };
				pInst = gameObjInstCreate(TYPE_OBJECT_COIN, &scale, &Pos, 0, 0.0f);
				++TotalCoins;
			}
		}

	//deprecated variables
	facingRight = true;
	wallJumpLeft = 0.0;
	wallJumpRight = 0.0;
}

// =========================================================
// 
// Handle input and set the physics variables accordingly
// 
// =========================================================
void Update_Input_Physics(void)
{
	/***********
	if Right arrow key is pressed
		Set hero velocity X to MOVE_VELOCITY_HERO
	else
	if Left arrow key is pressed
		Set hero velocity X to -MOVE_VELOCITY_HERO
	else
		Set hero velocity X to 0

	if Spacebar key is triggered AND Hero is colliding from the bottom
		Set hero velocity Y to JUMP_VELOCITY

	if Escape key is triggered
		Exit to main menu
	***********/
	


	//The following is a partial unfinished code

	if (AEInputCheckCurr(AEVK_RIGHT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		facingRight = true;
		pHero->velCurr.x = MOVE_VELOCITY_HERO;
	}
	else if (AEInputCheckCurr(AEVK_LEFT))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		facingRight = false;
		pHero->velCurr.x = -MOVE_VELOCITY_HERO;
	}
	else
	{
		pHero->velCurr.x = 0.0f;
	}

	if (AEInputCheckTriggered(AEVK_SPACE))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		if (pHero->gridCollisionFlag & COLLISION_BOTTOM)
			pHero->velCurr.y = JUMP_VELOCITY;
	}

	if (AEInputCheckTriggered(AEVK_ESCAPE))//DO NOT change or update or duplicate this input code line! Otherwise penalties may apply!
	{
		gGameStateNext = GS_MAINMENU;
	}
}

// =========================================================
// 
// Particle System
//  -- Particles creation with random properties
//  -- Do not exceed 100 particles running at once, per frame
//  -- Particles spawn relative to Hero's location
//  -- You can use "AERandFloat()" helper AE function	
// 
// =========================================================
void Hero_Particles_Creation(void)
{
	// Hero's Particle
	AEVec2 vel{ (1.25f + AERandFloat()) * (facingRight ? -1.0f : 1.0f), (AERandFloat() + 0.5f) * 5.0f };

	// offset from Hero
	AEVec2 offset(pHero->posCurr);
	offset.x += pHero->scale.x * (facingRight ? -0.25f : 0.25f);
	offset.y += pHero->scale.y * 0.5f;

	// particles created
	float uniformScaleValue = 0.25f + AERandFloat() * 0.25f;
	AEVec2 particleScale{ uniformScaleValue, uniformScaleValue };
	gameObjInstCreate(TYPE_OBJECT_PARTICLE, &particleScale, &offset, &vel, 0);
}

// =========================================================
// 
// Hero's wall jump
//  -- Deprecated behavior
// 
// =========================================================
void Hero_WallJump()
{
	// Wall Jump conditions
	if (pHero->velCurr.y > 0.0f)
	{
		if (GetCellValue(static_cast<int>(pHero->posCurr.x - (pHero->scale.x * 0.5f) - 0.1f),
			static_cast<int>(pHero->posCurr.y)) == TYPE_OBJECT_COLLISION)
		{
			wallJumpRight = 0.5;
		}
		if (GetCellValue(static_cast<int>(pHero->posCurr.x + (pHero->scale.x * 0.5f) + 0.1f),
			static_cast<int>(pHero->posCurr.y)) == TYPE_OBJECT_COLLISION)
		{
			wallJumpLeft = 0.5;
		}
	}

	// Wall Jump conditions Lifetime update
	if (wallJumpLeft > 0.0)
	{
		wallJumpLeft -= g_dt;
		if (wallJumpLeft <= 0.0)
			wallJumpLeft = 0.0;
	}
	if (wallJumpRight > 0.0)
	{
		wallJumpRight -= g_dt;
		if (wallJumpRight <= 0.0)
			wallJumpRight = 0.0;
	}

	// Wall Jump
	/*
	if (AEInputCheckTriggered(...) &&
		((wallJumpRight > 0.0 &&
			(AEInputCheckCurr(...))) ||
			(wallJumpLeft > 0.0 &&
				(AEInputCheckCurr(...)))))
	{
		wallJumpLeft = 0.0;
		wallJumpRight = 0.0;
		pHero->velCurr.y = JUMP_VELOCITY;
	}
	*/
}

// =========================================================
// 
// Update object instances physics-gravity
//  -- Gravity applied onto enemies and hero only
// 
// =========================================================
void Apply_GravityPhysics(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		/***********
		Apply gravity
			Velocity Y = Gravity * Frame Time + Velocity Y
		***********/
		if (pInst->pObject->type == TYPE_OBJECT_HERO ||
			pInst->pObject->type == TYPE_OBJECT_ENEMY)
		{
			pInst->velCurr.y += GRAVITY * (f32)g_dt;
		}
	}
}

// =========================================================
// 
// Particle System
//  -- Particles destruction based on its lifetime	
// 
// =========================================================
void Hero_Particles_Destruction(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	// Updates particle lifetime
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if (0 == (pInst->flag & FLAG_ACTIVE))
			continue;

		if (pInst->pObject->type == TYPE_OBJECT_PARTICLE)
		{
			//using the scale as a lifetime variable here
			pInst->scale.x -= (f32)g_dt * 0.45f;
			if (pInst->scale.x <= 0.01f)
			{
				gameObjInstDestroy(pInst);
			}
		}
	}	
}

// ======================================================================
// 
// update colliders of all active game object instances
//  -- Calculate the AABB bounding rectangle of the active instance, using the starting position:
//		boundingRect_min = -(BOUNDING_RECT_SIZE/2.0f) * instance->scale + instance->posPrev
//		boundingRect_max = +(BOUNDING_RECT_SIZE/2.0f) * instance->scale + instance->posPrev
//
// ======================================================================
void Update_BoundingBoxes(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		//bounding position are computed as the starting position in this game loop. Before computing the new position
		pInst->boundingBox.min.x = -(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.x + pInst->posCurr.x;
		pInst->boundingBox.min.y = -(BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.y + pInst->posCurr.y;
		pInst->boundingBox.max.x = (BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.x + pInst->posCurr.x;
		pInst->boundingBox.max.y = (BOUNDING_RECT_SIZE / 2.0f) * pInst->scale.y + pInst->posCurr.y;
	}
}

// ======================================================================
// 
// update physics of all active game object instances
//  -- New position of the active instance is updated here with the velocity calculated earlier
// 
// ======================================================================
void Update_Positions(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		//New positions of the instances are updated here with the velocity already calculated earlier
		AEVec2 vel;
		AEVec2Scale(&vel, &pInst->velCurr, (f32)g_dt);
		AEVec2Add(&pInst->posCurr, &pInst->posCurr, &vel);
	}
}

// ======================================================================
// 
// check for grid's binary collision
//  -- check hero and enemies against binary grid collision
// 
// ======================================================================
void Check_GridBinaryCollision(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object instances
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		/***********
		Update grid collision flag

		if collision from bottom
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from top
			Snap to cell on Y axis
			Velocity Y = 0

		if collision from left
			Snap to cell on X axis
			Velocity X = 0

		if collision from right
			Snap to cell on X axis
			Velocity X = 0
		***********/

		if (pInst->pObject->type == TYPE_OBJECT_HERO ||
			pInst->pObject->type == TYPE_OBJECT_ENEMY)
		{
			// Update object's collision with grid flag
			pInst->gridCollisionFlag = CheckInstanceBinaryMapCollision(pInst->posCurr.x, pInst->posCurr.y, pInst->scale.x, pInst->scale.y);

			// Top or Bottom side is in collision with background grid
			if (pInst->gridCollisionFlag & COLLISION_BOTTOM || pInst->gridCollisionFlag & COLLISION_TOP)
			{
				SnapToCell(&pInst->posCurr.y);
				pInst->velCurr.y = 0.0f;
			}
			// Left or Right side is in collision with background grid
			if (pInst->gridCollisionFlag & COLLISION_LEFT || pInst->gridCollisionFlag & COLLISION_RIGHT)
			{
				SnapToCell(&pInst->posCurr.x);
				pInst->velCurr.x = 0.0f;
			}
		}
	}
}

// ======================================================================
// 
// check for rectangle-rectangle collisions
// checking for dynamic collision among object instances:
//  -- Hero against enemies
//  -- Hero against coins
// 
// ======================================================================
void Update_AABBCollisions(void)
{
	/**********
	for each game object instance
		Skip if it's inactive or if it's non collidable

		If it's an enemy
			If collision between the enemy instance and the hero (rectangle - rectangle)
				Decrement hero lives
				Reset the hero's position in case it has lives left, otherwise RESTART the level

		If it's a coin
			If collision between the coin instance and the hero (rectangle - rectangle)
				Remove the coin and decrement the coin counter.
				Go to level2, in case no more coins are left and you are at Level1.
				Quit the game level to the main menu, in case no more coins are left and you are at Level2.
	**********/

	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// Skip non-active object or if it's non collidable
		if (0 == (pInst->flag & FLAG_ACTIVE) ||
			(pInst->flag & FLAG_NON_COLLIDABLE))
		{
			continue;
		}

		if (pInst->pObject->type == TYPE_OBJECT_ENEMY)
		{
			// Enemy collided with hero
			float firstTimeOfCollision = 0.0f;
			if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr, firstTimeOfCollision))
			{
				--HeroLives;
				if (HeroLives > 0)
				{
					pHero->posCurr.x = static_cast<f32>(Hero_Initial_X) + 0.5f;
					pHero->posCurr.y = static_cast<f32>(Hero_Initial_Y) + 0.5f;
				}
				else
				{
					gGameStateNext = GS_RESTART;
				}
			}
		}
		else if (pInst->pObject->type == TYPE_OBJECT_COIN)
		{
			// Coin collected
			float firstTimeOfCollision = 0.0f;
			if (CollisionIntersection_RectRect(pInst->boundingBox, pInst->velCurr, pHero->boundingBox, pHero->velCurr, firstTimeOfCollision))
			{
				gameObjInstDestroy(pInst);
				--TotalCoins;
				if (TotalCoins <= 0)
				{
					if (gGameStateCurr == GS_PLATFORM1)
						gGameStateNext = GS_PLATFORM2;
					else if (gGameStateCurr == GS_PLATFORM2)
						youWon = true;//gGameStateNext = GS_MAINMENU;
				}
			}
		}
	}
}

// =====================================================================
// 
// calculate the concatenated matrix transformation for all objects
// 
// =====================================================================
void Update_ObjectsTransformations(void)
{
	int i = 0;
	GameObjInst* pInst = nullptr;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		AEMtx33 scale, rot, trans;
		pInst = sGameObjInstList + i;
		
		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// Compute the scaling matrix
		// Compute the rotation matrix
		// Compute the translation matrix
		AEMtx33Scale(&scale, pInst->scale.x, pInst->scale.y);
		AEMtx33Rot(&rot, pInst->dirCurr);
		AEMtx33Trans(&trans, pInst->posCurr.x, pInst->posCurr.y);

		// Concatenate the 3 matrix in the correct order in the object instance's "transform" matrix
		AEMtx33Concat(&pInst->transform, &trans, &rot);
		AEMtx33Concat(&pInst->transform, &pInst->transform, &scale);
	}
}

// =====================================================================
// 
// update camera position
//  -- follows the player
//  -- clamps to the level's borders
// 
// =====================================================================
void Update_CameraPosition_Level2(void)
{
	// Update Camera World position, for Level 2 only!
		// To follow the player's position
		// To clamp the camera position at the level's borders, the clamp must be between (0,0) position
		// and (maxCamPosition.x, maxCamPosition.y) position that you need to calculate
			// You may use an alpha engine helper function to clamp the camera position: AEClamp()
				// Example: camPos.x = AEClamp(camPos.x, 0.0f, maxCamPosition.x);
		// To set the camera position use AEGfxSetCamPosition()

	AEVec2 camPos(pHero->posCurr);
	AEMtx33MultVec(&camPos, &MapTransform, &camPos);

	//calculate a cell dimension, in the viewport.
	AEVec2 cellDimension;
	AEVec2Set(&cellDimension,
		(f32)AEGfxGetWindowWidth() / (f32)GRID_WIDTH_IN_VIEWPORT,
		(f32)AEGfxGetWindowHeight() / (f32)GRID_HEIGHT_IN_VIEWPORT);

	//calculate maximum camera world position at the top-right
	AEVec2 maxCamPosition;
	AEVec2Set(&maxCamPosition,
		(cellDimension.x * (f32)(BINARY_MAP_WIDTH - GRID_WIDTH_IN_VIEWPORT)),
		(cellDimension.y * (f32)(BINARY_MAP_HEIGHT - GRID_HEIGHT_IN_VIEWPORT)));

	//clamp
	camPos.x = AEClamp(camPos.x, 0.0f, maxCamPosition.x);
	camPos.y = AEClamp(camPos.y, 0.0f, maxCamPosition.y);

	//apply camera position
	AEGfxSetCamPosition(camPos.x, camPos.y);
}

// =====================================================================
// 
// draw the tile map
//  -- it must be drawn before the dynamic objects
// 
// =====================================================================
void Draw_TileMap_BackgroundGrid(void)
{
	/*********
	for each array element in BinaryCollisionArray (2 loops)
		Each cell needs to be translated by its half width and
		height. That is why you should compute the cell's translation 
		matrix according to its X and Y coordinates and save it in 
		"cellTranslation"
		Concatenate MapTransform with the cell's transformation
		and save the result in "cellFinalTransformation"
		Send "cellFinalTransformation" matrix to the graphics manager using "AEGfxSetTransform"

		Draw the instance's shape depending on the cell's value using "AEGfxMeshDraw"
			Use the black instance in case the cell's value is TYPE_OBJECT_EMPTY
			Use the white instance in case the cell's value is TYPE_OBJECT_COLLISION
	*********/

	int i = 0, j = 0;

	AEMtx33 cellTranslation, cellFinalTransformation;

	UNREFERENCED_PARAMETER(cellTranslation);
	UNREFERENCED_PARAMETER(cellFinalTransformation);

#if defined(__EMSCRIPTEN__)
	// Web-only viewport culling: drawing every map cell each frame floods WebGL
	// with draw calls (cheap on a native GL driver, very expensive across the JS
	// boundary). Precompute the visible world rectangle once; cells outside it
	// are skipped below. Native build is unchanged.
	f32 aeCamX = 0.0f, aeCamY = 0.0f;
	AEGfxGetCamPosition(&aeCamX, &aeCamY);
	const f32 aeHalfW = (f32)AEGfxGetWindowWidth()  * 0.5f;
	const f32 aeHalfH = (f32)AEGfxGetWindowHeight() * 0.5f;
	const f32 aeCellW = (f32)AEGfxGetWindowWidth()  / (f32)GRID_WIDTH_IN_VIEWPORT;   // one-cell margin
	const f32 aeCellH = (f32)AEGfxGetWindowHeight() / (f32)GRID_HEIGHT_IN_VIEWPORT;
#endif

	for (i = 0; i < BINARY_MAP_WIDTH; ++i)
		for (j = 0; j < BINARY_MAP_HEIGHT; ++j)
		{
#if defined(__EMSCRIPTEN__)
			// Skip cells outside the camera view (uses the SAME MapTransform as render).
			{
				AEVec2 aeCellCenter, aeCellWorld;
				AEVec2Set(&aeCellCenter, static_cast<f32>(i) + 0.5f, static_cast<f32>(j) + 0.5f);
				AEMtx33MultVec(&aeCellWorld, &MapTransform, &aeCellCenter);
				if (aeCellWorld.x + aeCellW < aeCamX - aeHalfW ||
				    aeCellWorld.x - aeCellW > aeCamX + aeHalfW ||
				    aeCellWorld.y + aeCellH < aeCamY - aeHalfH ||
				    aeCellWorld.y - aeCellH > aeCamY + aeHalfH)
					continue;
			}
#endif
			// Concatenate the matrices here
			AEMtx33Trans(&cellTranslation, static_cast<f32>(i) + 0.5f, static_cast<f32>(j) + 0.5f);
			AEMtx33Concat(&cellFinalTransformation, &MapTransform, &cellTranslation);


			// Set the current object instance's transform matrix using "AEGfxSetTransform"
			AEGfxSetTransform(cellFinalTransformation.m);//DO NOT change or update or duplicate this code line! Otherwise penalties may apply!
			
			
			// Draw the shape used by the current object instance using "AEGfxMeshDraw" here
			if (BinaryCollisionArray[j][i] == TYPE_OBJECT_EMPTY)
				AEGfxMeshDraw(pBlackInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
			else if (BinaryCollisionArray[j][i] == TYPE_OBJECT_COLLISION)
				AEGfxMeshDraw(pWhiteInstance->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
		}
}

// ----------------------------------------------------------------------------
//
// draw all dynamic object instances in the list
//  -- it must be drawn after the tile map background
//
// ----------------------------------------------------------------------------
void Draw_DynamicObjectsInstances(void)
{
	/******REMINDER*****
	You need to concatenate MapTransform with the transformation matrix
	of any object you want to draw. MapTransform transform the instance
	from the normalized coordinates system of the binary map

	For each active and visible object instance
		Concatenate MapTransform with its transformation matrix
		and save the result in "objectFinalTransformation"
		Send "objectFinalTransformation" matrix to the graphics manager using "AEGfxSetTransform"
		Draw the instance's shape using "AEGfxMeshDraw"
	*******************/

	AEMtx33 objectFinalTransformation;
	UNREFERENCED_PARAMETER(objectFinalTransformation);

	int i = 0;

	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// Skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0 || (pInst->flag & FLAG_VISIBLE) == 0)
			continue;

		// Apply any additional rendering setup - if needed here
		// Apply Tint colour on particle
		if (pInst->pObject->type == TYPE_OBJECT_PARTICLE)
		{
			AEGfxSetBlendMode(AE_GFX_BM_BLEND);
			f32 alpha = AEClamp(pInst->scale.x * 2.0f, 0.0f, 1.0f);
			
			AEGfxSetColorToMultiply(0.1f, 0.3f, AERandFloat(), alpha);
		}
		else
		{
			AEGfxSetBlendMode(AE_GFX_BM_NONE);
			AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
		}


		// Concatenate the MapTransform matrix with the transformation of each game object instance here
		AEMtx33Concat(&objectFinalTransformation, &MapTransform, &pInst->transform);


		// Set the current object instance's transform matrix using "AEGfxSetTransform"
		AEGfxSetTransform(objectFinalTransformation.m);//DO NOT change or update or duplicate this code line! Otherwise penalties may apply!


		// Draw the shape used by the current object instance using "AEGfxMeshDraw"
		AEGfxMeshDraw(pInst->pObject->pMesh, AE_GFX_MDM_TRIANGLES);
	}
}

// ----------------------------------------------------------------------------
//
// display game stats using in game text objects
//
// ----------------------------------------------------------------------------
void Display_GameStats(void)
{
	char strBuffer[100];
	memset(strBuffer, 0, 100 * sizeof(char));
	

	if (youWon)
	{
		sprintf_s(strBuffer, "Elly's my dream LOVE :-)");
		AEGfxPrint(fontId, strBuffer, -1.0f, 0.1f, 3.0f, 1.0f, 0.0f, 0.0f, 1.0f);
	}
	else
	{
		sprintf_s(strBuffer, "Coins Left:  %i", TotalCoins);
		AEGfxPrint(fontId, strBuffer, -0.9f, 0.9f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);

		sprintf_s(strBuffer, "FPS:  %i", static_cast<int>(AEFrameRateControllerGetFrameRate() + 0.5));
		AEGfxPrint(fontId, strBuffer, -0.1f, 0.9f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f);

		sprintf_s(strBuffer, "Lives:  %i", HeroLives);
		AEGfxPrint(fontId, strBuffer, 0.6f, 0.9f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f);
	}
}

// ----------------------------------------------------------------------------
//
// destroy all object instances in the array using "gameObjInstDestroy"
//
// ----------------------------------------------------------------------------
void Destroy_ObjectsInstances(void)
{
	// destroy all object in the list
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
		gameObjInstDestroy(sGameObjInstList + i);
}

// ----------------------------------------------------------------------------
//
// free all mesh data (shapes) of each object using "AEGfxTriFree"
// free objects data and map data
//
// ----------------------------------------------------------------------------
void Free_AllAllocatedMemory(void)
{
	// free all the created meshes
	for (u32 i = 0; i < sGameObjNum; i++)
		AEGfxMeshFree(sGameObjList[i].pMesh);

	// free objects data
	free(sGameObjList);
	free(sGameObjInstList);

	// free the map data
	FreeMapData();
}

// ----------------------------------------------------------------------------
//
// this state machine is used by an enemy to move left and right on a platform.
//
// ----------------------------------------------------------------------------
void EnemyStateMachine(GameObjInst* pInst)
{
	/***********
	This state machine has 2 states: STATE_GOING_LEFT and STATE_GOING_RIGHT
	Each state has 3 inner states: INNER_STATE_ON_ENTER, INNER_STATE_ON_UPDATE, INNER_STATE_ON_EXIT
	Use "switch" statements to determine which state and inner state the enemy is currently in.


	STATE_GOING_LEFT
		INNER_STATE_ON_ENTER
			Set velocity X to -MOVE_VELOCITY_ENEMY
			Set inner state to "on update"

		INNER_STATE_ON_UPDATE
			If collision on left side OR bottom left cell is non collidable
				Initialize the counter to ENEMY_IDLE_TIME
				Set inner state to on exit
				Set velocity X to 0
				Snap to cell the position along the x-axis


		INNER_STATE_ON_EXIT
			Decrement counter by frame time
			if counter is less than 0 (sprite's idle time is over)
				Set state to "going right"
				Set inner state to "on enter"

	STATE_GOING_RIGHT is basically the same, with few modifications.

	***********/

	switch (pInst->state)
	{
	case STATE_GOING_LEFT:

		switch (pInst->innerState)
		{
		case INNER_STATE_ON_ENTER:
			pInst->velCurr.x = -MOVE_VELOCITY_ENEMY;
			pInst->innerState = INNER_STATE_ON_UPDATE;
			break;
		case INNER_STATE_ON_UPDATE:
			if ((pInst->gridCollisionFlag & COLLISION_LEFT) ||
				GetCellValue(static_cast<int>(pInst->posCurr.x - (pInst->scale.x * 0.5f)), static_cast<int>(pInst->posCurr.y) - 1) != TYPE_OBJECT_COLLISION)
			{
				pInst->idle_counter = ENEMY_IDLE_TIME;
				pInst->innerState = INNER_STATE_ON_EXIT;
				pInst->velCurr.x = 0.0f;
				SnapToCell(&pInst->posCurr.x);
			}
			break;
		case INNER_STATE_ON_EXIT:
			pInst->idle_counter -= static_cast<double>(g_dt);
			if (pInst->idle_counter < 0.0)
			{
				pInst->state = STATE_GOING_RIGHT;
				pInst->innerState = INNER_STATE_ON_ENTER;
			}
			break;
		}
		break;
	case STATE_GOING_RIGHT:

		switch (pInst->innerState)
		{
		case INNER_STATE_ON_ENTER:
			pInst->velCurr.x = MOVE_VELOCITY_ENEMY;
			pInst->innerState = INNER_STATE_ON_UPDATE;
			break;
		case INNER_STATE_ON_UPDATE:
			if ((pInst->gridCollisionFlag & COLLISION_RIGHT) ||
				GetCellValue(static_cast<int>(pInst->posCurr.x + (pInst->scale.x * 0.5f)), static_cast<int>(pInst->posCurr.y) - 1) != TYPE_OBJECT_COLLISION)
			{
				pInst->idle_counter = ENEMY_IDLE_TIME;
				pInst->innerState = INNER_STATE_ON_EXIT;
				pInst->velCurr.x = 0.0f;
				SnapToCell(&pInst->posCurr.x);
			}
			break;
		case INNER_STATE_ON_EXIT:
			pInst->idle_counter -= static_cast<double>(g_dt);
			if (pInst->idle_counter < 0.0)
			{
				pInst->state = STATE_GOING_LEFT;
				pInst->innerState = INNER_STATE_ON_ENTER;
			}
			break;
		}
		break;

	case STATE_NONE:		// idle / no-movement state - nothing to update
	default:
		break;
	}
}

// ----------------------------------------------------------------------------
//
//	This function retrieves the value of the element (X;Y) in BinaryCollisionArray.
//	Before retrieving the value, it should check that the supplied X and Y values
//	are not out of bounds (in that case return 0)
//
// ----------------------------------------------------------------------------
int GetCellValue(int X, int Y)
{
	if (0 <= X && X < BINARY_MAP_WIDTH &&
		0 <= Y && Y < BINARY_MAP_HEIGHT)
	{
		return BinaryCollisionArray[Y][X];
	}
	return 0;
}

// ----------------------------------------------------------------------------
//
//	This function creates 2 hot spots on each side of the object instance, 
//	and checks if each of these hot spots is in a collision area (which means 
//	the cell if falls in has a value of 1).
//	At the beginning of the function, a "Flag" integer should be initialized to 0.
//	Each time a hot spot is in a collision area, its corresponding bit 
//	in "Flag" is set to 1.
//	Finally, the function returns the integer "Flag"
//	The position of the object instance is received as PosX and PosY
//	The size of the object instance is received as scaleX and scaleY
//
//	Note: This function assume the object instance's size is 1 by 1 
//		  (the size of 1 tile)
//
//	Creating the hotspots:
//		-Handle each side separately.
//		-2 hot spots are needed for each collision side.
//		-These 2 hot spots should be positioned on 1/4 above the center 
//		and 1/4 below the center
//
//	Example: Finding the hots spots on the left side of the object instance
//
//	float x1, y1, x2, y2;
//
//	-hotspot 1
//	x1 = PosX + scaleX/2	To reach the right side
//	y1 = PosY + scaleY/4	To go up 1/4 of the height
//	
//	-hotspot 2
//	x2 = PosX + scaleX/2	To reach the right side
//	y2 = PosY - scaleY/4	To go down 1/4 of the height
//
// ----------------------------------------------------------------------------
int CheckInstanceBinaryMapCollision(float PosX, float PosY, float scaleX, float scaleY)
{
	int flag{ 0 };
	float lft_x1, lft_y1, lft_x2, lft_y2;
	float rht_x1, rht_y1, rht_x2, rht_y2;
	float top_x1, top_y1, top_x2, top_y2;
	float bot_x1, bot_y1, bot_x2, bot_y2;

	// LEFT hotspots
	lft_x1 = PosX - scaleX / 2.0f;
	lft_y1 = PosY + scaleY / 4.0f;
	lft_x2 = PosX - scaleX / 2.0f;
	lft_y2 = PosY - scaleY / 4.0f;
	if (GetCellValue(static_cast<int>(lft_x1), static_cast<int>(lft_y1)) ||
		GetCellValue(static_cast<int>(lft_x2), static_cast<int>(lft_y2)))
	{
		flag |= COLLISION_LEFT;
	}

	// RIGHT hotspots
	rht_x1 = PosX + scaleX / 2.0f;
	rht_y1 = PosY + scaleY / 4.0f;
	rht_x2 = PosX + scaleX / 2.0f;
	rht_y2 = PosY - scaleY / 4.0f;
	if (GetCellValue(static_cast<int>(rht_x1), static_cast<int>(rht_y1)) ||
		GetCellValue(static_cast<int>(rht_x2), static_cast<int>(rht_y2)))
	{
		flag |= COLLISION_RIGHT;
	}

	// TOP hotspots
	top_x1 = PosX - scaleX / 4.0f;
	top_y1 = PosY + scaleY / 2.0f;
	top_x2 = PosX + scaleX / 4.0f;
	top_y2 = PosY + scaleY / 2.0f;
	if (GetCellValue(static_cast<int>(top_x1), static_cast<int>(top_y1)) ||
		GetCellValue(static_cast<int>(top_x2), static_cast<int>(top_y2)))
	{
		flag |= COLLISION_TOP;
	}

	// DOWN hotspots
	bot_x1 = PosX - scaleX / 4.0f;
	bot_y1 = PosY - scaleY / 2.0f;
	bot_x2 = PosX + scaleX / 4.0f;
	bot_y2 = PosY - scaleY / 2.0f;
	if (GetCellValue(static_cast<int>(bot_x1), static_cast<int>(bot_y1)) ||
		GetCellValue(static_cast<int>(bot_x2), static_cast<int>(bot_y2)))
	{
		flag |= COLLISION_BOTTOM;
	}

	return flag;
}

// ----------------------------------------------------------------------------
//
//	This function snaps the value sent as parameter to the center of the cell.
//	It is used when a sprite is colliding with a collision area from one 
//	or more side.
//	To snap the value sent by "Coordinate", find its integral part by type 
//	casting it to an integer, then add 0.5 (which is half the cell's width 
//	or height)
//
// ----------------------------------------------------------------------------
void SnapToCell(float* Coordinate)
{
	*Coordinate = static_cast<int>(*Coordinate) + 0.5f;
}

// ----------------------------------------------------------------------------
//
//	This function opens the file name "FileName" and retrieves all the map data.
//	It allocates memory for the 2 arrays: MapData & BinaryCollisionArray
//	The first line in this file is the width of the map.
//	The second line in this file is the height of the map.
//	The remaining part of the file is a series of numbers
//	Each number represents the ID (or value) of a different element in the 
//	double dimensionaly array.
//
//	Example:
//
//	Width 5
//	Height 5
//	1 1 1 1 1
//	1 1 1 3 1
//	1 4 2 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//
//	After importing the above data, "MapData" and " BinaryCollisionArray" 
//	should be
//
//	1 1 1 1 1
//	1 1 1 3 1
//	1 4 2 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//	and
//
//	1 1 1 1 1
//	1 1 1 0 1
//	1 0 0 0 1
//	1 0 0 0 1
//	1 1 1 1 1
//
//	respectively.
//	
//	Finally, the function returns 1 if the file named "FileName" exists, 
//	otherwise it returns 0
//
// ----------------------------------------------------------------------------
int ImportMapDataFromFile(const char* FileName)
{
	std::fstream inputFile(FileName, std::ios_base::in);
	if (!inputFile)
	{
		return 0;
	}

	std::string temp;
	inputFile >> temp >> BINARY_MAP_WIDTH >> temp >> BINARY_MAP_HEIGHT;

	// memory allocation
	MapData = new int* [BINARY_MAP_HEIGHT];
	BinaryCollisionArray = new int* [BINARY_MAP_HEIGHT];
	for (int i{ 0 }; i < BINARY_MAP_HEIGHT; ++i)
	{
		MapData[i] = new int[BINARY_MAP_WIDTH];
		BinaryCollisionArray[i] = new int[BINARY_MAP_WIDTH];
	}

	// storing input
	for (int i{ 0 }; i < BINARY_MAP_HEIGHT; ++i)
	{
		for (int j{ 0 }; j < BINARY_MAP_WIDTH; ++j)
		{
			inputFile >> MapData[i][j];
			BinaryCollisionArray[i][j] = MapData[i][j] == 1 ? 1 : 0;
		}
	}

	inputFile.close();
	return 1;
}

// ----------------------------------------------------------------------------
//
//	This function frees the memory that was allocated for the 2 arrays MapData 
//	& BinaryCollisionArray which was allocated in the "ImportMapDataFromFile" 
//	function
//
// ----------------------------------------------------------------------------
void FreeMapData(void)
{
	for (int i{ 0 }; i < BINARY_MAP_HEIGHT; ++i)
	{
		delete[] MapData[i];
		delete[] BinaryCollisionArray[i];
	}
	delete[] MapData;
	delete[] BinaryCollisionArray;
}

// ----------------------------------------------------------------------------
//
//	This function prints out the content of the 2D array �MapData�
//	You must print to the console, the same information you are reading from "Exported.txt" file
//	Follow exactly the same format of the file, including the print of the width and the height
//	Add spaces and end lines at convenient places
//
// ----------------------------------------------------------------------------
void PrintRetrievedInformation(void)
{
	//to console
	std::cout << "Width: " << BINARY_MAP_WIDTH << "\n";
	std::cout << "Height: " << BINARY_MAP_HEIGHT << "\n";
	for (int i{ 0 }; i < BINARY_MAP_HEIGHT; ++i)
	{
		for (int j{ 0 }; j < BINARY_MAP_WIDTH; ++j)
		{
			std::cout << MapData[i][j] << " ";
		}
		std::cout << "\n";
	}
}
