/* Start Header ************************************************************************/
/*!
\file		GameState_Platform_Extension.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file declares important structures, enumerations, helper functions and
			essential functions for the platform game state implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_GAMESTATE_PLATFORM_EXTENSION_H_
#define CSD1130_GAMESTATE_PLATFORM_EXTENSION_H_

#include "AEEngine.h"
#include <math.h>
#include "AABB_Collision.h"
#include "GameStateConstants.h"
#include "Main.h"


// ----------------------------------------------------------------------------
//
// object flag definition
//
// ----------------------------------------------------------------------------
enum TYPE_OBJECT
{
	TYPE_OBJECT_EMPTY,			//0
	TYPE_OBJECT_COLLISION,		//1
	TYPE_OBJECT_HERO,			//2
	TYPE_OBJECT_ENEMY,			//3
	TYPE_OBJECT_COIN,			//4
	TYPE_OBJECT_PARTICLE		//5  	//ADDED_NEW
};

// ----------------------------------------------------------------------------
//
// state machine states
//
// ----------------------------------------------------------------------------
enum STATE
{
	STATE_NONE,
	STATE_GOING_LEFT,
	STATE_GOING_RIGHT
};

// ----------------------------------------------------------------------------
//
// state machine inner states
//
// ----------------------------------------------------------------------------
enum INNER_STATE
{
	INNER_STATE_ON_ENTER,
	INNER_STATE_ON_UPDATE,
	INNER_STATE_ON_EXIT
};

// ----------------------------------------------------------------------------
//
// Game object structure
//
// ----------------------------------------------------------------------------
struct GameObj
{
	unsigned long		type;		// object type
	AEGfxVertexList *	pMesh;		// This will hold the triangles which will form the shape of the object
};

// ----------------------------------------------------------------------------
//
// Game object instance structure
//
// ----------------------------------------------------------------------------
struct GameObjInst
{
	GameObj * pObject;						// pointer to the 'original'
	unsigned int	flag;					// bit flag or-ed together
	AEVec2			scale;					// scaling value of the object instance
	AEVec2			posCurr;				// object current position
	AEVec2			velCurr;				// object current velocity
	float			dirCurr;				// object current direction

	AEVec2			posPrev;				// object previous position -> it's the position calculated in the previous loop

	AEMtx33			transform;				// object transformation matrix: Each frame, 
											// calculate the object instance's transformation matrix and save it here.

	AABB			boundingBox;			// object bouding box that encapsulates the object

	int				gridCollisionFlag;		// used to hold the current object's collision flag status

	void* pUserData;						// pointer to custom data specific for each object type

	enum			STATE state;			// state of the object instance
	enum			INNER_STATE innerState;	// inner State of the object instance		
	double			idle_counter;			// general purpose counter (this variable will be used in the enemy state machine for the idle time counting)

	//For Particle System
	float			lifetime;				// particle's lifetime value if this instance is a particle
};


// ----------------------------------------------------------------------------
//
// Externs
//
// ----------------------------------------------------------------------------
extern int				HeroLives;
extern int				Hero_Initial_X;
extern int				Hero_Initial_Y;
extern int				TotalCoins;
extern GameObj *		sGameObjList;
extern unsigned int		sGameObjNum;
extern GameObjInst *	sGameObjInstList;
extern unsigned int		sGameObjInstNum;
extern int **			MapData;
extern int **			BinaryCollisionArray;
extern int				BINARY_MAP_WIDTH;
extern int				BINARY_MAP_HEIGHT;
extern GameObjInst *	pBlackInstance;
extern GameObjInst *	pWhiteInstance;
extern AEMtx33			MapTransform;
extern GameObjInst *	pHero;
extern bool				youWon;


void Import_MapData(void);
void Compute_MapTransformMatrix(void);
void Starting_GameObjectsInstances(void);
void Update_Input_Physics(void);
void Hero_Particles_Creation(void);
void Hero_WallJump(void);//deprecated behavior
void Apply_GravityPhysics(void);
void Hero_Particles_Destruction(void);
void Update_BoundingBoxes(void);
void Update_Positions(void);
void Check_GridBinaryCollision(void);
void Update_AABBCollisions(void);
void Update_ObjectsTransformations(void);
void Update_CameraPosition_Level2(void);
void Draw_TileMap_BackgroundGrid(void);
void Draw_DynamicObjectsInstances(void);
void Display_GameStats(void);
void Destroy_ObjectsInstances(void);
void Free_AllAllocatedMemory(void);
void EnemyStateMachine(GameObjInst* pInst);


// ----------------------------------------------------------------------------
//
// Helper functions
//
// ----------------------------------------------------------------------------
GameObjInst* gameObjInstCreate( unsigned long type,
								AEVec2* scale,
								AEVec2* pPos,
								AEVec2* pVel,
								float dir);
void gameObjInstDestroy(GameObjInst* pInst);


// ----------------------------------------------------------------------------
//
// Assignment 3 Part 1 - APIs
//
// ----------------------------------------------------------------------------
int		GetCellValue(int X, int Y);
int		CheckInstanceBinaryMapCollision(float PosX, float PosY,
										float scaleX, float scaleY);
void	SnapToCell(float* Coordinate);
int		ImportMapDataFromFile(const char* FileName);
void	FreeMapData(void);
void	PrintRetrievedInformation(void);

#endif // CSD1130_GAMESTATE_PLATFORM_EXTENSION_H_


