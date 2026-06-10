/* Start Header ************************************************************************/
/*!
\file		GameState_Platform.cpp
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file implements the main six functions of the Platform game state.
			The functions are used for both gameplay levels of the platformer game.
			It also contains some helper functions.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameState_Platform.h"

void Load_AllMeshes(void);


// ----------------------------------------------------------------------------
//
// Variables and containers to use
//
// ----------------------------------------------------------------------------
int				HeroLives = 0;
int				Hero_Initial_X = 0;
int				Hero_Initial_Y = 0;
int				TotalCoins = 0;

// list of original objects
GameObj	*		sGameObjList = nullptr;
unsigned int	sGameObjNum = 0;

// list of object instances
GameObjInst *	sGameObjInstList = nullptr;
unsigned int	sGameObjInstNum = 0;

//Binary map data
int **			MapData = nullptr;
int **			BinaryCollisionArray = nullptr;
int				BINARY_MAP_WIDTH = 0;
int				BINARY_MAP_HEIGHT = 0;
GameObjInst	*	pBlackInstance = nullptr;
GameObjInst	*	pWhiteInstance = nullptr;
AEMtx33			MapTransform;

//We need a pointer to the hero's instance for faster access
GameObjInst *	pHero = nullptr;


// ----------------------------------------------------------------------------
//
// "Load" function of this state
// This function loads all necessary assets for Level 1 or Level 2
// It should be called once before the start of the level
//
// ----------------------------------------------------------------------------
void GameStatePlatformLoad(void)
{
	sGameObjList = (GameObj *)calloc(GAME_OBJ_NUM_MAX, sizeof(GameObj));
	sGameObjInstList = (GameObjInst *)calloc(GAME_OBJ_INST_NUM_MAX, sizeof(GameObjInst));
	sGameObjNum = 0;

	// Helper function to load all meshes
	Load_AllMeshes();
	
	// Helper function to load the current level's map data
	Import_MapData();

	// Helper function to compute MapTransform matrix
	Compute_MapTransformMatrix();	
}

// ----------------------------------------------------------------------------
//
// "Initialize" function of this game state. Accounts for Level 1 and Level 2.
// This function is used to prepare the state's data in order to be used for the
// first time. If a state is restarted or reset, this cycle function is called 
// once.
//
// ----------------------------------------------------------------------------
void GameStatePlatformInit(void)
{
	g_fixedDT = 0.01667;
	g_dt = g_fixedDT;

	// Helper function to create all the initial game objects instances
	Starting_GameObjectsInstances();
}

// ----------------------------------------------------------------------------
//
// "Update" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformUpdate(void)
{
	int i = 0;
	GameObjInst *pInst = nullptr;

	UNREFERENCED_PARAMETER(pInst);

	Update_Input_Physics();

	Hero_Particles_Creation();

	// Deprecated behavior
	if(0)
	{
		Hero_WallJump();
	}
	
	Apply_GravityPhysics();
	
	Hero_Particles_Destruction();


	/***********
	//Update enemies state machine
	***********/
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; ++i)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		// Update enemy behaviour
		if (pInst->pObject->type == TYPE_OBJECT_ENEMY)
		{
			EnemyStateMachine(pInst);
		}
	}
	
	
	/***********
	Save previous positions
	  -- For all instances
	***********/
	for (i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		pInst = sGameObjInstList + i;

		// skip non-active object
		if ((pInst->flag & FLAG_ACTIVE) == 0)
			continue;

		pInst->posPrev.x = pInst->posCurr.x;
		pInst->posPrev.y = pInst->posCurr.y;
	}


	Update_BoundingBoxes();

	Update_Positions();

	Check_GridBinaryCollision();
	
	Update_AABBCollisions();

	Update_ObjectsTransformations();

	if (gGameStateCurr == GS_PLATFORM2)
	{
		Update_CameraPosition_Level2();
	}
}

// ----------------------------------------------------------------------------
//
// "Draw" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformDraw(void)
{	
	//Drawing setup
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxTextureSet(NULL, 0, 0);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);


	if (!youWon)
	{
		//Drawing the tile map
		Draw_TileMap_BackgroundGrid();

		//Drawing the dynamic object instances
		Draw_DynamicObjectsInstances();
	}

	//Drawing setup
	AEGfxSetBlendMode(AE_GFX_BM_NONE);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);


	//Display total coins and hero lives texts
	Display_GameStats();
}

// ----------------------------------------------------------------------------
//
// "Free" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformFree(void)
{
	Destroy_ObjectsInstances();
}

// ----------------------------------------------------------------------------
//
// "Unload" function of this game state. Accounts for Level 1 and Level 2.
//
// ----------------------------------------------------------------------------
void GameStatePlatformUnload(void)
{
	Free_AllAllocatedMemory();
}

// =========================================================
// 
// creates all needed meshes
// 
// =========================================================
void Load_AllMeshes(void)
{
	GameObj* pObj;

	//Creating the black object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_EMPTY;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF000000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF000000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the white object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_COLLISION;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the hero object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_HERO;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFF0000FF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the enemy object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_ENEMY;


	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFF0000, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFF0000, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the Coin object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_COIN;


	AEGfxMeshStart();
	//Creating the circle shape
	int Parts = 12;
	for (float i = 0; i < Parts; ++i)
	{
		AEGfxTriAdd(
			0.0f, 0.0f, 0xFFFFFF00, 0.0f, 0.0f,
			cosf(i * 2 * PI / Parts) * 0.5f, sinf(i * 2 * PI / Parts) * 0.5f, 0xFFFFFF00, 0.0f, 0.0f,
			cosf((i + 1) * 2 * PI / Parts) * 0.5f, sinf((i + 1) * 2 * PI / Parts) * 0.5f, 0xFFFFFF00, 0.0f, 0.0f);
	}

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");


	//Creating the Particle object
	pObj = sGameObjList + sGameObjNum++;
	pObj->type = TYPE_OBJECT_PARTICLE;

	AEGfxMeshStart();
	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	AEGfxTriAdd(
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);

	pObj->pMesh = AEGfxMeshEnd();
	AE_ASSERT_MESG(pObj->pMesh, "fail to create object!!");
}

// ----------------------------------------------------------------------------
//
// A helper function to create a game object instance
//
// ----------------------------------------------------------------------------
GameObjInst* gameObjInstCreate( unsigned long type, 
								AEVec2* scale,
								AEVec2* pPos, 
								AEVec2* pVel, 
								float dir)
{
	AEVec2 zero;
	AEVec2Zero(&zero);

	AE_ASSERT_PARM(type < sGameObjNum);
	
	// loop through the object instance list to find a non-used object instance
	for (unsigned int i = 0; i < GAME_OBJ_INST_NUM_MAX; i++)
	{
		GameObjInst* pInst = sGameObjInstList + i;

		// check if current instance is not used
		if (pInst->flag == 0)
		{
			// it is not used => use it to create the new instance
			pInst->pObject			 = sGameObjList + type;
			pInst->flag				 = FLAG_ACTIVE | FLAG_VISIBLE;
			pInst->scale			 = *scale;
			pInst->posCurr			 = pPos ? *pPos : zero;
			pInst->velCurr			 = pVel ? *pVel : zero;
			pInst->dirCurr			 = dir;
			pInst->pUserData		 = 0;
			pInst->gridCollisionFlag = 0;
			pInst->state			 = STATE_GOING_LEFT;
			pInst->innerState		 = INNER_STATE_ON_ENTER;
			pInst->idle_counter		 = 0;
			pInst->lifetime		     = 0.0f;

			// return the newly created instance
			return pInst;
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
//
// A helper function to destroy a game object instance
//
// ----------------------------------------------------------------------------
void gameObjInstDestroy(GameObjInst* pInst)
{
	// if instance is destroyed before, just return
	if (pInst->flag == 0)
		return;

	// zero out the flag
	pInst->flag = 0;
}
