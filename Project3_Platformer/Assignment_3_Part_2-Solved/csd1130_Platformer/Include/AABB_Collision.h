/* Start Header ************************************************************************/
/*!
\file		AABB_Collision.h
\author		DigiPen, DigiPen, DigiPen
\par		digipen@digipen.edu
\date		January, 22, 2025
\brief		This file declares the static and dynamic collision functions, as well as
			the main AABB-AABB collision function implementation.

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#ifndef CSD1130_COLLISION_H_
#define CSD1130_COLLISION_H_

#include "AEEngine.h"

// ----------------------------------------------------------------------------
//
// Externs
//
// ----------------------------------------------------------------------------
extern f64					g_fixedDT;
extern f64					g_dt;
extern f64					g_appTime;


// ----------------------------------------------------------------------------
//
// AABB collider structure
//
// ----------------------------------------------------------------------------
struct AABB
{
	AEVec2	min;
	AEVec2	max;
};

bool CollisionIntersection_RectRect_Static( const AABB& aabb1,					//Input
											const AABB& aabb2);					//Input


bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,					//Input
											const AEVec2& vel1,					//Input 
											const AABB& aabb2,					//Input 
											const AEVec2& vel2,					//Input
											float& firstTimeOfCollision);		//Output: the calculated value of tFirst

// ----------------------------------------------------------------------------
//
// Collision test between 2 rectangles using AABB-AABB collision algorithm.
// First check "Step 1", is the static intersection check => return intersection 
// in case both rectangles are already overlapping.
// Second check "Step 2 until 5", in case the first check fails, we apply a more 
// dynamic collision test to see if any rectangle's movement may lead to intersection 
// in this game loop.
// returns true if there is intersection.
//
// ----------------------------------------------------------------------------
inline bool CollisionIntersection_RectRect( const AABB& aabb1,					//Input
											const AEVec2& vel1,					//Input 
											const AABB& aabb2,					//Input 
											const AEVec2& vel2,					//Input
											float& firstTimeOfCollision)		//Output: return the calculated value of tFirst
{
	//Step 1
	bool staticCollision = false;
	staticCollision = CollisionIntersection_RectRect_Static(aabb1, aabb2);
	if (staticCollision)
	{
		return true;
	}

	//Step 2 until 5
	return CollisionIntersection_RectRect_Dynamic(aabb1, vel1, aabb2, vel2, firstTimeOfCollision);
}

#endif // CSD1130_COLLISION_H_