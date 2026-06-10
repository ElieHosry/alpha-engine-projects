// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AELineSegment2.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2012/03/19
// Purpose			:	implementation of the 2D line segment library
// History			:
// ---------------------------------------------------------------------------

#include "AEExport.h"
#include "AETypes.h"
#include "AEVec2.h"
#include "AELineSegment2.h"
//#include "AEMath.h"
#include <math.h>


// ---------------------------------------------------------------------------


AE_API s32 AEBuildLineSegment2(AELineSegment2 *pLS, AEVec2 *Point0, AEVec2 *pPt1)
{
	if( (fabs(Point0->x - pPt1->x)) < EPSILON && (fabs(Point0->y - pPt1->y)) < EPSILON )
		return 0;

	pLS->mP0 = *Point0;
	pLS->mP1 = *pPt1;

	AEVec2 e;									// Edge
	AEVec2Sub(&e, pPt1, Point0);

	AEVec2Set(&pLS->mN, e.y, -e.x);				// Normal
	AEVec2Normalize(&pLS->mN, &pLS->mN);

	pLS->mNdotP0 = AEVec2DotProduct(&pLS->mN, &pLS->mP0);

	return 1;
}


// ---------------------------------------------------------------------------
