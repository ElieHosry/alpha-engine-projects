/* Start Header ************************************************************************/
/*!
\file		AABB_Collision.cpp
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

#include "AABB_Collision.h"
#include "Main.h"

// ----------------------------------------------------------------------------
//
// Globals
//
// ----------------------------------------------------------------------------
// fixed dt value to force it
f64					g_fixedDT = 0.01667;

// stores the game loop time that you must use in all your physics calculations
f64					g_dt = g_fixedDT;

// stores to total application time until the current loop
f64					g_appTime = 0.0;


// ----------------------------------------------------------------------------
//
// Static AABB collision:
// AABB collision between two rectangles that are possibly already overlapping
// This is a rigid check, considering both rectangles are not moving
// If this function finds intersection between the two rectangles, there will be
// no need to call the dynamic collision check.
// return true if there is overlap.
//
// ----------------------------------------------------------------------------
bool CollisionIntersection_RectRect_Static(const AABB& aabb1,				//Input
	const AABB& aabb2)				//Input
{
	/*
	Implement the dynamic collision intersection over here.

	The steps "from the lecture slides" are :

	Step 1 : Check for static collision detection between rectangles(static : before moving).
		If the check returns no overlap, you continue with the dynamic collision test
		with the subsequent next steps 2 to 5 (dynamic : with velocities).
		Otherwise you return collision is true, and you stop.
	*/

	bool staticCollision = true;
	if ((aabb1.max.x < aabb2.min.x) || (aabb1.min.x > aabb2.max.x))
		staticCollision = false;
	if ((aabb1.max.y < aabb2.min.y) || (aabb1.min.y > aabb2.max.y))
		staticCollision = false;
	return staticCollision;
}

// ----------------------------------------------------------------------------
//
// Dynamic AABB collision:
// AABB collision between two rectangles, that we already know they are not 
// overlapping.
// Both rectangles may have velocities and the function must return the first
// time of intersection if there is any.
// return true if there is intersection.
//
// ----------------------------------------------------------------------------
bool CollisionIntersection_RectRect_Dynamic(const AABB& aabb1,				//Input
	const AEVec2& vel1,				//Input 
	const AABB& aabb2,				//Input 
	const AEVec2& vel2,				//Input
	float& firstTimeOfCollision)	//Output: the calculated value of tFirst
{
	/*
	Implement the dynamic collision intersection over here.

	The steps "from the lecture slides" are:

	Step 2: Initialize and calculate the new "relative" velocity of Vb
			tFirst = 0  //tFirst variable is commonly used for both the x-axis and y-axis
			tLast = dt  //tLast variable is commonly used for both the x-axis and y-axis

	Step 3: Working with one dimension (x-axis).
			if(Vb < 0)
				case 1
				case 4
			else if(Vb > 0)
				case 2
				case 3
			else //(Vb == 0)
				case 5

			case 6

	Step 4: Repeat step 3 on the y-axis

	Step 5: Return true: the rectangles intersect
	*/


	//Step 2
	AEVec2	vel;//relative velocity
	vel.x = vel2.x - vel1.x;
	vel.y = vel2.y - vel1.y;

	//float tFirst = 0.0;
	firstTimeOfCollision = 0.0f;
	float tLast = (float)g_dt;
	float tmp;

	//Step 3: test along x-axis
	if (vel.x < 0.0f)
	{
		if (aabb2.max.x < aabb1.min.x)
		{
			return false;
		}

		if (aabb1.max.x < aabb2.min.x)
		{
			tmp = (aabb1.max.x - aabb2.min.x) / vel.x;

			firstTimeOfCollision = max(tmp, firstTimeOfCollision);
		}
		if (aabb2.max.x > aabb1.min.x)
		{
			tmp = (aabb1.min.x - aabb2.max.x) / vel.x;

			tLast = min(tmp, tLast);
		}
	}
	else if (vel.x > 0.0f)
	{
		if (aabb2.min.x > aabb1.max.x)
		{
			return false;
		}

		if (aabb1.min.x > aabb2.max.x)
		{
			tmp = (aabb1.min.x - aabb2.max.x) / vel.x;

			firstTimeOfCollision = max(tmp, firstTimeOfCollision);
		}
		if (aabb1.max.x > aabb2.min.x)
		{
			tmp = (aabb1.max.x - aabb2.min.x) / vel.x;

			tLast = min(tmp, tLast);
		}
	}
	else //if (vel.x == 0.0f)
	{
		if (aabb2.max.x < aabb1.min.x)
		{
			return false;
		}
		else if (aabb2.min.x > aabb1.max.x)
		{
			return false;
		}
	}

	//case 6
	if (firstTimeOfCollision > tLast)
	{
		return 0;
	}


	//Step 4: test along y-axis
	if (vel.y < 0.0f)
	{
		if (aabb2.max.y < aabb1.min.y)
		{
			return false;
		}

		if (aabb1.max.y < aabb2.min.y)
		{
			tmp = (aabb1.max.y - aabb2.min.y) / vel.y;

			firstTimeOfCollision = max(tmp, firstTimeOfCollision);
		}
		if (aabb2.max.y > aabb1.min.y)
		{
			tmp = (aabb1.min.y - aabb2.max.y) / vel.y;

			tLast = min(tmp, tLast);
		}
	}
	else if (vel.y > 0.0f)
	{
		if (aabb2.min.y > aabb1.max.y)
		{
			return false;
		}

		if (aabb2.max.y < aabb1.min.y)
		{
			tmp = (aabb1.min.y - aabb2.max.y) / vel.y;

			firstTimeOfCollision = max(tmp, firstTimeOfCollision);
		}
		if (aabb1.max.y > aabb2.min.y)
		{
			tmp = (aabb1.max.y - aabb2.min.y) / vel.y;

			tLast = min(tmp, tLast);
		}
	}
	else // if(vel.y == 0.0f)
	{
		if (aabb2.max.y < aabb1.min.y)
		{
			return false;
		}
		else if (aabb2.min.y > aabb1.max.y)
		{
			return false;
		}
	}

	//case 6
	if (firstTimeOfCollision > tLast)
	{
		return false;
	}

	return true;
}