// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEMath.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/01//31
// Purpose			:	Implementations of the math library
// History			:
// - 2008/01/31		:	- initial implementation
// ---------------------------------------------------------------------------

#include "AEExport.h"
#include "AETypes.h"
#include <math.h>
#include "AEMath.h"

// -----------------------------------------------------------------------
/*
AE_API void AEMathInit(void)
{
	// allocate the sine and cosine table
	// allocate the asine and acosine table
}
*/
// ---------------------------------------------------------------------------

AE_API f32 AEDegToRad(f32 x)
{
	return x * (PI / 180.0f);
}

// ---------------------------------------------------------------------------

AE_API f32 AERadToDeg(f32 x)
{
	return x * (180.0f / PI);
}

// ---------------------------------------------------------------------------

AE_API f32 AESin(f32 x)
{
	return sinf(x);
}

// ---------------------------------------------------------------------------

AE_API f32 AECos(f32 x)
{
	return cosf(x);
}

// ---------------------------------------------------------------------------

AE_API f32 AETan(f32 x)
{
	return tanf(x);
}

// ---------------------------------------------------------------------------

AE_API f32 AEASin(f32 x)
{
	return asinf(x);
}

// ---------------------------------------------------------------------------

AE_API f32 AEACos(f32 x)
{
	return acosf(x);
}

// ---------------------------------------------------------------------------

AE_API f32 AEATan(f32 x)
{
	return atanf(x);
}

// ---------------------------------------------------------------------------

AE_API u32 AEIsPowOf2(u32 x)
{
	return ((x) && (((x) & ((x) - 1)) == 0));
}

// ---------------------------------------------------------------------------

AE_API u32 AENextPowOf2(u32 x)
{
	x |= x >>  1;
	x |= x >>  2;
	x |= x >>  4;
	x |= x >>  8;
	x |= x >> 16;

	return x + 1;
}

// ---------------------------------------------------------------------------

AE_API u32 AELogBase2(u32 x)
{
	u32 i;

	for(x = x >> 1, i = 0; x >> i; i++)
		;

	return i;
}

// ---------------------------------------------------------------------------

AE_API f32 AEClamp(f32 X, f32 Min, f32 Max)
{
	if (X < Min)
		return Min;
	if (X > Max)
		return Max;
	return X;
}

// ---------------------------------------------------------------------------

AE_API f32 AEWrap(f32 x, f32 x0, f32 x1)
{
	if (x < x0)
		return x + (x1 - x0);
	if (x > x1)
		return x - (x1 - x0);
	return x;
}

// ---------------------------------------------------------------------------

AE_API f32 AEMin(f32 x, f32 y)
{
	return (x < y) ? x : y;
}

// ---------------------------------------------------------------------------

AE_API f32 AEMax(f32 x, f32 y)
{
	return (x < y) ? y : x;
}

// ---------------------------------------------------------------------------

AE_API s32 AEInRange(f32 x, f32 x0, f32 x1)
{
	if ((x0 <= x) && (x <= x1))
		return 1;

	return 0;
}

#if 0
// ---------------------------------------------------------------------------

AE_API f32 AECalcDistPointToCircle(AEVec2* pPos, AEVec2* pCtr, f32 radius)
{
//AE_ASSERT(pPos && pCtr && (radius >= 0.0f));

	return AEVec2Distance(pPos, pCtr) - radius;
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistPointToRect(AEVec2* pPos, AEVec2* pRect, f32 sizeX, f32 sizeY)
{
//AE_ASSERT(pPos && pRect && (sizeX >= 0.0f) && (sizeY >= 0.0f));

	AEVec2 p, q;

	// move the point to the rect space
	AEVec2Sub(&p, pPos, pRect);

	// given size is the total rect width/height, half them for the calculation
	sizeX *= 0.5f;
	sizeY *= 0.5f;

	// check if the given point is outside of the rect
	if ((fabs(p.x) > sizeX) || (fabs(p.y) > sizeY))
	{
		// find a point on the rect that's closest to the given point
		q.x = AEClamp(p.x, -sizeX, sizeX);
		q.y = AEClamp(p.y, -sizeY, sizeY);

		// distance from closest point to the given point is the distance to the rect
		return AEVec2Distance(&p, &q);
	}

	// given point is inside the rectangle
	// => find the edge the given point is closest to
	p.x = (p.x >= 0.0f) ? (sizeX - p.x) : (sizeX + p.x);
	p.y = (p.y >= 0.0f) ? (sizeY - p.y) : (sizeY + p.y);

	// distance to the closest edge is the distance to the rect
	// * negate the result to indicate that the point is inside
	return -((p.x < p.y) ? p.x : p.y);
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistPointToLineSeg(AEVec2* pPos, AEVec2* pLine0, AEVec2* pLine1)
{
	AEVec2	u, v;
	f32		uLen, wLen;

	AEVec2Sub(&u, pLine1, pLine0);
	AEVec2Sub(&v, pPos, pLine0);

	// get the length of the line segment
	uLen = AEVec2Length(&u);

	// if the line segment is practically a point, 
	// return the distance from the point to the start of the line segment
	if (uLen < EPSILON)
		return AEVec2Distance(pPos, pLine0);

	// calculate the distance from the start of the line segment to the
	// projection of the point on the line segment
	wLen = AEVec2DotProduct(&u, &v) / uLen;

	// if the projection is 'behind' the start of the line segment, 
	// return the distance to the start of the segment
	if (wLen <= 0.0f)
		return AEVec2Length(&v);
	// if the projection is 'beyond' the end of the line segment, 
	// return the distance to the end of the segment
	else if (wLen >= uLen)
		return AEVec2Distance(pPos, pLine1);

	// projection is in the line segment
	// => calculate the distance from the point to the line segment
	wLen = AEVec2SquareLength(&v) - wLen * wLen;

	// just in case the floating point error cause wLen to be less than 0
	// => should never happend!!
	if (wLen <= 0.0f)
		return 0.0f;

	// return the distance
	return sqrtf(wLen);
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistPointToConvexPoly(AEVec2* pPos, AEVec2* pVtx, u32 vtxNum)
{
	pPos = 0;
	pVtx = 0;
	vtxNum = 0;

	return -1.0f;
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistCircleToCircle(AEVec2* pCtr0, f32 radius0, AEVec2* pCtr1, f32 radius1)
{
//AE_ASSERT(pCtr0 && pCtr1 && (radius0 >= 0.0f) && (radius1 >= 0.0f));

	return AEVec2Distance(pCtr0, pCtr1) - radius0 - radius1;
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistCircleToRect(AEVec2* pCtr, f32 radius, AEVec2* pRect, f32 sizeX, f32 sizeY)
{
	return AECalcDistPointToRect(pCtr, pRect, sizeX, sizeY) - radius;
}

// ---------------------------------------------------------------------------

AE_API f32 AECalcDistRectToRect(AEVec2* pRect0, f32 sizeX0, f32 sizeY0, AEVec2* pRect1, f32 sizeX1, f32 sizeY1, AEVec2* pNormal)
{
	AEVec2	pos, nrm;
	f32		d, minD = FLT_MAX;

	AEVec2Sub(&pos, pRect1, pRect0);

	if ((fabs(pos.x) >= (sizeX0 + sizeX1) * 0.5f) || (fabs(pos.y) >= (sizeY0 + sizeY1) * 0.5f))
		return 0.0f;

	// check if 2nd rect is to the right of the 1st one
	if (pos.x >= 0.0f)
	{
		minD = pos.x - sizeX0 * 0.5f - sizeX1 * 0.5f;
		AEVec2Set(&nrm, -1.0f, 0.0);
	}
	// 2nd rect is to the left to the 1st one
	else
	{
		minD = -pos.x - sizeX0 * 0.5f - sizeX1 * 0.5f;
		AEVec2Set(&nrm, 1.0f, 0.0);
	}

	// check if 2nd rect is above of the 1st one
	if (pos.y >= 0.0f)
	{
		d = pos.y - sizeY0 * 0.5f - sizeY1 * 0.5f;

		if (d > minD)
		{
			minD = d;
			AEVec2Set(&nrm, 0.0f, -1.0f);
		}
	}
	// 2nd rect is under the 1st one
	else
	{
		d = -pos.y - sizeY0 * 0.5f - sizeY1 * 0.5f;

		if (d > minD)
		{
			minD = d;
			AEVec2Set(&nrm, 0.0f, 1.0);
		}
	}

	if (pNormal)
		*pNormal = nrm;

	return minD;
}

// ---------------------------------------------------------------------------

AE_API s32 AETestPointToCircle(AEVec2* pPos, AEVec2* pCtr, f32 radius)
{
//AE_ASSERT(pPos && pCtr && (radius >= 0.0f));

	return (AEVec2Distance(pPos, pCtr) <= radius);
}

// ---------------------------------------------------------------------------

AE_API s32 AETestPointToRect(AEVec2* pPos, AEVec2* pRect, f32 sizeX, f32 sizeY)
{
//AE_ASSERT(pPos && pRect && (sizeX >= 0.0f) && (sizeY >= 0.0f));

	AEVec2 p;

	// move the point to the rect space
	AEVec2Sub(&p, pPos, pRect);

	// given size is the total rect width/height, half them for the calculation
	sizeX *= 0.5f;
	sizeY *= 0.5f;

	if ((p.x > sizeX) || (p.x < -sizeX) || (p.y > sizeY) || (p.y < -sizeY))
		return 0;

	return 1;
}

// ---------------------------------------------------------------------------

AE_API s32 AETestCircleToCircle(AEVec2* pCtr0, f32 radius0, AEVec2* pCtr1, f32 radius1)
{
//AE_ASSERT(pCtr0 && pCtr1 && (radius0 >= 0.0f) && (radius1 >= 0.0f));

	return (AEVec2Distance(pCtr0, pCtr1) <= (radius0 + radius1));
}

// ---------------------------------------------------------------------------

AE_API s32 AETestCircleToRect(AEVec2* pCtr, f32 radius, AEVec2* pRect, f32 sizeX, f32 sizeY)
{
	return AECalcDistPointToRect(pCtr, pRect, sizeX, sizeY) < radius;
}

// ---------------------------------------------------------------------------

AE_API s32 AETestRectToRect(AEVec2* pRect0, f32 sizeX0, f32 sizeY0, AEVec2* pRect1, f32 sizeX1, f32 sizeY1)
{
	AEVec2	pos;

	AEVec2Sub(&pos, pRect1, pRect0);

	sizeX0 *= 0.5f;
	sizeY0 *= 0.5f;
	sizeX1 *= 0.5f;
	sizeY1 *= 0.5f;

	if ((pos.x >  (sizeX0 + sizeX1)) || 
		(pos.x < -(sizeX0 + sizeX1)) || 
		(pos.y >  (sizeY0 + sizeY1)) || 
		(pos.y < -(sizeY0 + sizeY1)))
		return 0;

	return 1;
}

// ---------------------------------------------------------------------------

AE_API f32 AEStaticPointToStaticLineSegment(AEVec2 *pPos, AELineSegment2 *pLine)
{
	return AEVec2DotProduct(&pLine->mN, pPos) - pLine->mNdotP0;
}

// ---------------------------------------------------------------------------

AE_API f32 AEAnimatedPointToStaticLineSegment(AEVec2 *pStart, AEVec2 *pEnd, AELineSegment2 *pLine, AEVec2 *pInter)
{
	// Non-Collision test 1/5 - Both in inside plane
	if( (AEVec2DotProduct(&pLine->mN, pStart) < pLine->mNdotP0) && (AEVec2DotProduct(&pLine->mN, pEnd) < pLine->mNdotP0) )
		return -1.0f;


	// Non-Collision test 2/5 - Both in outside plane
	if( (AEVec2DotProduct(&pLine->mN, pStart) > pLine->mNdotP0) && (AEVec2DotProduct(&pLine->mN, pEnd) > pLine->mNdotP0) )
		return -1.0f;


	// Non-Collision test 3/5 - Point moving on a parallel line
	AEVec2 v;
	AEVec2Sub(&v, pEnd, pStart);
	if( fabs(AEVec2DotProduct(&pLine->mN, &v)) < EPSILON )
		return -1.0f;


	// Intersection: Calculate time
	f32 t;
	t = ( AEVec2DotProduct(&pLine->mN, &pLine->mP0) - AEVec2DotProduct(&pLine->mN, pStart) )  / ( AEVec2DotProduct(&pLine->mN, &v) );
	//printf("Collision time: %f\n", t);

	// Intersection with infinite line: Compute intersection point
	AEVec2 pI;
	AEVec2ScaleAdd(&pI, &v, pStart, t);


	// Non-Collision test 4/5 - Intersection point:  P-------P0-------P1---
	AEVec2 p0ph, p0p1;
	AEVec2Sub(&p0ph, &pI, &pLine->mP0);
	AEVec2Sub(&p0p1, &pLine->mP1, &pLine->mP0);

	if( AEVec2DotProduct(&p0ph, &p0p1) < 0.0f) 
		return -1.0f;


	// Non-Collision test 5/5 - Intersection point:  ---P0-------P1-------P0
	AEVec2 p1ph, p1p0;
	AEVec2Sub(&p1ph, &pI, &pLine->mP1);
	AEVec2Sub(&p1p0, &pLine->mP0, &pLine->mP1);

	if( AEVec2DotProduct(&p1ph, &p1p0) < 0.0f) 
		return -1.0f;


	*pInter = pI;
	return t;
}

// ---------------------------------------------------------------------------

AE_API f32 AEAnimatedCircleToStaticLineSegment(AEVec2 *pStart, AEVec2 *pEnd, f32 radius, AELineSegment2 *pLine, AEVec2 *pInter)
{
	// Non-Collision test 1/5 - Both in inside plane
	if( ((AEVec2DotProduct(&pLine->mN, pStart) - pLine->mNdotP0) < -radius ) && ((AEVec2DotProduct(&pLine->mN, pEnd) - pLine->mNdotP0) < -radius) )
		return -1.0f;


	// Non-Collision test 2/5 - Both in outside plane
	if( ((AEVec2DotProduct(&pLine->mN, pStart) - pLine->mNdotP0) > radius) && ((AEVec2DotProduct(&pLine->mN, pEnd) - pLine->mNdotP0) > radius) )
		return -1.0f;


	// Non-Collision test 3/5 - Point moving on a parallel line
	AEVec2 v;
	AEVec2Sub(&v, pEnd, pStart);
	if( fabs(AEVec2DotProduct(&pLine->mN, &v)) < EPSILON )
		return -1.0f;


	// Intersection: Calculate time
	f32 t;
	f32 D = (AEStaticPointToStaticLineSegment(pStart, pLine) < 0) ? -radius : radius; 
	t = ( AEVec2DotProduct(&pLine->mN, &pLine->mP0) - AEVec2DotProduct(&pLine->mN, pStart) + D)  / ( AEVec2DotProduct(&pLine->mN, &v) );
	//printf("Collision time: %f\n", t);

	// Intersection: Compute intersection point
	AEVec2 pI;
	AEVec2ScaleAdd(&pI, &v, pStart, t);


	// Non-Collision test 4/5 - Intersection point:  P-------P0-------P1---
	AEVec2 p0ph, p0p1;
	AEVec2Sub(&p0ph, &pI, &pLine->mP0);
	AEVec2Sub(&p0p1, &pLine->mP1, &pLine->mP0);

	if( AEVec2DotProduct(&p0ph, &p0p1) < 0.0f) 
		return -1.0f;


	// Non-Collision test 5/5 - Intersection point:  ---P0-------P1-------P0
	AEVec2 p1ph, p1p0;
	AEVec2Sub(&p1ph, &pI, &pLine->mP1);
	AEVec2Sub(&p1p0, &pLine->mP0, &pLine->mP1);

	if( AEVec2DotProduct(&p1ph, &p1p0) < 0.0f) 
		return -1.0f;


	*pInter = pI;
	return t;
}


// ---------------------------------------------------------------------------


AE_API f32 AEReflectAnimatedPointOnStaticLineSegment(AEVec2 *pStart, AEVec2 *pEnd, AELineSegment2 *pLine, AEVec2 *pInter, AEVec2 *pReflect)

{
	f32 t = AEAnimatedPointToStaticLineSegment(pStart, pEnd, pLine, pInter);

	if(t > 0.0)
	{
		AEVec2 i, s;
		f32 iDotn;

		AEVec2Sub(&i, pEnd, pInter);
		iDotn = AEVec2DotProduct(&i, &pLine->mN);
		AEVec2Scale(&s, &pLine->mN, iDotn);

		AEVec2ScaleAdd(pReflect, &s, &i, -2.0f);
	}

	return t;
}



// ---------------------------------------------------------------------------

AE_API f32 AEReflectAnimatedCircleOnStaticLineSegment(AEVec2 *pStart, AEVec2 *pEnd, f32 radius, AELineSegment2 *pLine, AEVec2 *pInter, AEVec2 *pReflect)
{
	f32 t = AEAnimatedCircleToStaticLineSegment(pStart, pEnd, radius, pLine, pInter);

	if(t > 0.0f)
	{
		AEVec2 i, s;
		f32 iDotn;

		AEVec2Sub(&i, pEnd, pInter);
		iDotn = AEVec2DotProduct(&i, &pLine->mN);
		AEVec2Scale(&s, &pLine->mN, iDotn);

		AEVec2ScaleAdd(pReflect, &s, &i, -2.0f);
	}

	return t;
}

// ---------------------------------------------------------------------------

AE_API f32 AEAnimatedPointToStaticCircle(AEVec2 *pStart, AEVec2 *pEnd, AEVec2 *pCtr, f32 radius, AEVec2 *pInter)
{
	AEVec2 v, vn, psc;
	f32 m, n2, psc2, r2, s, ti0;

	AEVec2Sub(&psc, pCtr, pStart);
	AEVec2Sub(&v, pEnd, pStart);
	AEVec2Normalize(&vn, &v);

	m = AEVec2DotProduct(&psc, &vn);

	psc2 = AEVec2SquareDistance(pStart, pCtr);
	r2 = radius * radius;

	// Non Collision: pStart outside circle and moving away from it
	if(m < 0 && psc2 < r2 )
		return -1.0f;


	// Non Collision: Ray completely outside the circle
	n2 = psc2 - (m * m);
	if(n2 > r2)
		return -1.0f;

	// Compute collision time ti;
	s = sqrtf(r2 - n2);
	ti0 = (m - s) / AEVec2Length(&v);
	AEVec2ScaleAdd(pInter, &v, pStart, ti0);


	AEVec2 pepi, peps;
	AEVec2Sub(&pepi, pInter, pEnd);
	AEVec2Sub(&peps, pStart, pEnd);

	if(AEVec2DotProduct(&peps, &pepi) < 0)
		return -1.0f;

	return ti0;
}

// ---------------------------------------------------------------------------

AE_API f32 AEReflectAnimatedPointOnStaticCircle(AEVec2 *pStart, AEVec2 *pEnd, AEVec2 *pCtr, f32 radius, AEVec2 *pInter, AEVec2 *pReflect)
{
	f32 t = AEAnimatedPointToStaticCircle(pStart, pEnd, pCtr, radius, pInter);

	if(t > 0.0f)
	{
		AEVec2 m, n;
		f32 mDotn;

		AEVec2Sub(&n, pInter, pCtr);
		AEVec2Normalize(&n, &n);
		AEVec2Sub(&m, pStart, pInter);

		mDotn = AEVec2DotProduct(&m, &n);
		AEVec2ScaleSub(pReflect, &n, &m, 2.0f * mDotn);
	}

	return t;
}

// ---------------------------------------------------------------------------

AE_API f32 AEAnimatedCircleToStaticCircle(AEVec2 *pCtr0s, AEVec2 *pCtr0e, f32 radius0, AEVec2 *pCtr1, f32 radius1, AEVec2 *pInter)
{
	return AEAnimatedPointToStaticCircle(pCtr0s, pCtr0e, pCtr1, radius0 + radius1, pInter);
}

// ---------------------------------------------------------------------------

AE_API f32 AEReflectAnimatedCircleOnStaticCircle(AEVec2 *pCtr0s, AEVec2 *pCtr0e, f32 radius0, AEVec2 *pCtr1, f32 radius1, AEVec2 *pInter, AEVec2 *pReflect)
{
	return AEReflectAnimatedPointOnStaticCircle(pCtr0s, pCtr0e, pCtr1, radius0 + radius1, pInter, pReflect);
}

// ---------------------------------------------------------------------------

/*
AE_API f32 AESweepCircleToPoint(AEVec2* pCtr0, AEVec2* pCtr1, f32 radius, AEVec2* pP)
{
	AEVec2	u, v;
	f32		uLen, vLen, wLen, wLenPerp, t;

	AEVec2Sub(&u, pCtr1, pCtr0);
	AEVec2Sub(&v, pP, pCtr0);

	uLen = AEVec2Length(&u);

	// check if the circle is actually moving
	if (uLen < EPSILON)
	{
		// circle is not moving 
		// => check if the distance to the given point is less than the radius
		if (AEVec2Distance(pCtr0, pP) <= radius)
			return 0.0f;

		// circle is not moving and given point is 'far' => not touching
		return 2.0f;
	}

	// get distance from the start position to the given point
	vLen = AEVec2Length(&v);

	// if distance from start position to given point is less than radius
	// => point is in the radius
	if (vLen <= radius)
		return 0.0f;

	// calculate the length of the projection of v onto u
	wLen = AEVec2DotProduct(&u, &v) / uLen;

	// calculate the distance from the given point to its projection 
	// on the circle path
	wLenPerp = vLen * vLen - wLen * wLen;

	// check if the distance if greater than the radius
	if (wLenPerp > (radius * radius))
		return 2.0f;

	// calculate the distance from the start position to the position of the
	// circle when it touches the point for the 1st time along the path
	t = wLen - sqrtf(radius * radius - wLenPerp);

	if ((0.0f <= t) && (t <= uLen))
		return t / uLen;

	return 2.0f;
}

// ---------------------------------------------------------------------------

AE_API f32 AESweepCircleToLineSeg(AEVec2* pCtr0, AEVec2* pCtr1, f32 radius, AEVec2* pP0, AEVec2* pP1, AEVec2* pN)
{
#if 1

	AEVec2 p, u, v, n;
	f32    uLen, t, d, t1;

	// if the normal pointer is not given, point it to the normal variable
	if (pN == NULL)
		pN = &n;

	AEVec2Sub(&u, pP1,   pP0);
	AEVec2Sub(&v, pCtr1, pCtr0);

	uLen = AEVec2Length(&u);

	// check if the line segment is really just a point
	if (uLen < EPSILON)
		return AESweepCircleToPoint(pCtr0, pCtr1, radius, pP0);

	AEVec2Scale(&u, &u, 1.0f / uLen);

	// calculate the line segment plane equations
	n.x = -u.y;
	n.y =  u.x;
	d   = -AEVec2DotProduct(&n, pP0);

	t  = AEVec2DotProduct(&n, pCtr0) + d;
	t1 = AEVec2DotProduct(&n, pCtr1) + d;

	if (((t < -radius) && (t1 < -radius)) || ((t > radius) && (t1 > radius)))
		return 2.0f;

	t = AEVec2DotProduct(&n, &v);

	if (t < -EPSILON)
	{
		t = (AEVec2DotProduct(&n, pCtr0) + d - radius) / -t;
	}
	else if (t > EPSILON)
	{
		t = (AEVec2DotProduct(&n, pCtr0) + d + radius) / -t;

		n.x = -n.x;
		n.y = -n.y;
	}
	else
	{
		return 2.0f;
	}

	// calculate the circle position at time t
	AEVec2ScaleAdd(&p, &v, pCtr0, t);
	AEVec2Sub     (&v, &p, pP0);

	// check if the circle is before or beyond the line segment
	if (AEVec2DotProduct(&u, &v) < 0.0f)
	{
		t = AESweepCircleToPoint(pCtr0, pCtr1, radius, pP0);

		if ((t < 0.0f) || (t > 1.0f))
			return 2.0f;

		AEVec2Lerp	(&p, pCtr0, pCtr1, t);
		AEVec2Sub	(&u, &p, pP0);
		uLen = AEVec2Length(&u);

		if (uLen < EPSILON)
			*pN = n;
		else
			AEVec2Scale(pN, &u, 1.0f / uLen);

		return t;
	}


	if (AEVec2DotProduct(&u, &v) > uLen)
	{
		t = AESweepCircleToPoint(pCtr0, pCtr1, radius, pP1);

		if ((t < 0.0f) || (t > 1.0f))
			return 2.0f;

		AEVec2Lerp	(&p, pCtr0, pCtr1, t);
		AEVec2Sub	(&u, &p, pP1);
		uLen = AEVec2Length(&u);

		if (uLen < EPSILON)
			*pN = n;
		else
			AEVec2Scale(pN, &u, 1.0f / uLen);

		return t;
	}

	if ((t < 0.0f) || (t > 1.0f))
		return 2.0f;

	// copy the normal
	*pN = n;

	return t;

#else

	AEVec2	p, u, v, w, n;
	f32		uLen, t0, t1, a, b, c, d;
	s32	resultFlag = 1;

	// if the normal pointer is not given, point it to the normal variable
	if (pN == NULL)
		pN = &n;

	AEVec2Sub(&u, pP1,   pP0);
	AEVec2Sub(&v, pCtr1, pCtr0);

	// check if the line segment is really just a point
	if ((uLen = AEVec2Length(&u)) < EPSILON)
		return AESweepCircleToPoint(pCtr0, pCtr1, radius, pP0);

	AEVec2Sub(&w, pCtr0, pP0);

	f32 uu = AEVec2DotProduct(&u, &u), 
		uv = AEVec2DotProduct(&u, &v), 
		uw = AEVec2DotProduct(&u, &w), 
		vv = AEVec2DotProduct(&v, &v), 
		vw = AEVec2DotProduct(&v, &w), 
		ww = AEVec2DotProduct(&w, &w);

	// calculate the coeeficient for the quadratic equations
	a = vv - uv * uv / uu;
	b = 2.0f * (vw - uv * uw / uu);
	c = ww - uw * uw / uu - radius * radius;

	// flip the sign if 'a' is negative
	if (a < 0.0f)
	{
		a = -a;
		b = -b;
		c = -c;
	}

	// solve the quadratic equation
	if (a > EPSILON)
	{
		d = b * b - 4.0f * a * c;

		if (d >= 0.0f)
		{
			t0 = (-b - sqrtf(d)) / (2.0f * a);
			t1 = (-b + sqrtf(d)) / (2.0f * a);
		}
		else
		{
			resultFlag = false;
		}
	}
	else if (b > EPSILON)
	{
		t0 = t1 = -c / b;
	}
	else if (fabs(c) < EPSILON)
	{
		t0 = t1 = 0.0f;
	}
	else
	{
		resultFlag = false;
	}

	// if the equation cannot be solved, return no collision
	if (resultFlag == false)
		return 2.0f;

	// if the earlier intersection is beyond the circle path or 
	// the later intersection is before the circle path, return no collision
	if ((1.0f < t0) || (t1 < 0.0f))
		return 2.0f;

	// if earlier intersection 
	if (t0 < 0.0f)
		t0 = t1;

	if (1.0f < t0)
		return 0.0f;

	if ((0.0f <= t0) && (t0 <= 1.0f))
	{
	}
	else if ((

		// calculate the circle position at time t
		AEVec2ScaleAdd(&p, &v, pCtr0, t);
	AEVec2Sub     (&v, &p, pP0);

	// check if the circle is before or beyond the line segment
	if (AEVec2DotProduct(&u, &v) < 0.0f)
	{
		t = AESweepCircleToPoint(pCtr0, pCtr1, radius, pP0);

		if ((t < 0.0f) || (t > 1.0f))
			return 2.0f;

		AEVec2Lerp	(&p, pCtr0, pCtr1, t);
		AEVec2Sub	(&u, &p, pP0);
		uLen = AEVec2Length(&u);

		if (uLen < EPSILON)
			*pN = n;
		else
			AEVec2Scale(pN, &u, 1.0f / uLen);

		return t;
	}
	if (AEVec2DotProduct(&u, &v) > uLen)
	{
		t = AESweepCircleToPoint(pCtr0, pCtr1, radius, pP1);

		if ((t < 0.0f) || (t > 1.0f))
			return 2.0f;

		AEVec2Lerp	(&p, pCtr0, pCtr1, t);
		AEVec2Sub	(&u, &p, pP1);
		uLen = AEVec2Length(&u);

		if (uLen < EPSILON)
			*pN = n;
		else
			AEVec2Scale(pN, &u, 1.0f / uLen);

		return t;
	}

	if ((t < 0.0f) || (t > 1.0f))
		return 2.0f;

	// copy the normal
	*pN = n;

	return t;

#endif

}

// ---------------------------------------------------------------------------

//Sweeps a moving point against a static line
//Returns the time of intersection
AE_API f32 AESweepPointToLine(AEVec2 *pPos, AEVec2 *pVel, AEVec2 *pPnt, AEVec2 *pDirection)
{
	f32 D, n_Dot_v;
	AEVec2 normal;

	normal.x = pDirection->y;
	normal.y = -pDirection->x;

	//calculating constant of the normal line equation
	D = AEVec2DotProduct(pPnt, &normal);

	n_Dot_v = AEVec2DotProduct(&normal, pVel);

	if(n_Dot_v)
		//calculating the collision time
		return (D - AEVec2DotProduct(&normal, pPos)) / (AEVec2DotProduct(&normal, pVel));

	return -1.0f;
}


// ---------------------------------------------------------------------------

//Sweeps a moving circle against a static line
AE_API f32 AESweepCircleToLine(AEVec2 *pCtr, f32 radius, AEVec2 *pVel, AEVec2 *pPnt, AEVec2 *pDirection)
{
	f32 D, n_Dot_v;

	AEVec2 normalizedNormal;

	//Normalizing the normal of the line
	normalizedNormal.x = pDirection->y;
	normalizedNormal.y = -pDirection->x;
	AEVec2Normalize(&normalizedNormal, &normalizedNormal);

	//calculating constant of the normal line equation
	D = AEVec2DotProduct(pPnt, &normalizedNormal) - radius;

	n_Dot_v = AEVec2DotProduct(&normalizedNormal, pVel);

	if(n_Dot_v)
		//calculating the collision time
		return (D - AEVec2DotProduct(&normalizedNormal, pCtr)) / (AEVec2DotProduct(&normalizedNormal, pVel));

	return -1.0f;
}

// ---------------------------------------------------------------------------

//Reflects a moving point on a static line
AE_API s32 AEReflectPointOnLine(AEVec2 *pPos, AEVec2 *pVel, AEVec2 *pPnt, AEVec2 *pDirection, AEVec2 *pNewPosition, AEVec2 *pNewVelocity)
{
	f32 t;

	t = AESweepPointToLine(pPos, pVel, pPnt, pDirection);

	//Checking if the point actually collided with the line
	if(t > 0.0f && t < 1.0f)
	{
		AEVec2 pReflect, M, collisionPoint, normal;

		//Calculating the line normal
		normal.x = -pDirection->y;
		normal.y = pDirection->x;

		//Finding the collision point
		AEVec2ScaleAdd(&collisionPoint, pVel, pPos, t);

		//Computing the reflected amount
		t = 1 - t;

		//Computing the reflected vector pReflect
		AEVec2Scale(&M, &normal, -AEVec2DotProduct(pVel, &normal) / AEVec2DotProduct(&normal, &normal));
		AEVec2Scale(&M, &M, 2.0f);

		//Computing pReflect
		AEVec2Add(&pReflect, pVel, &M);

		//Computing the final position after reflection
		AEVec2ScaleAdd(pNewPosition, &pReflect, &collisionPoint, t);

		//Saving the new velocity
		if(pNewVelocity)
			AEVec2Set(pNewVelocity, pReflect.x, pReflect.y);

		return 1;
	}

	//Calculating the new position, although there was no collision
	AEVec2ScaleAdd(pNewPosition, pVel, pPos, t);
	return 0;
}

// ---------------------------------------------------------------------------


//Reflects a moving circle on a static line. Returns 0 if there is no
//collision between the circle and the line.
AE_API s32 AEReflectCircleOnLine(AEVec2 *pCtr, f32 radius, AEVec2 *pVel, AEVec2 *pPnt, AEVec2 *pDirection, AEVec2 *pNewPosition, AEVec2 *pNewVelocity)
{
	f32 t;

	//Getting the collision time
	t = AESweepCircleToLine(pCtr, radius, pVel, pPnt, pDirection);

	//Checking if the circle collided with the line
	if(t > 0.0f && t < 1.0f)
	{
		AEVec2 pReflect, M, collisionPoint, normal;

		//Calculating the line normal
		normal.x = -pDirection->y;
		normal.y = pDirection->x;

		//Finding the collision point
		AEVec2ScaleAdd(&collisionPoint, pVel, pCtr, t);

		//Computing the reflected amount
		t = 1 - t;

		//Computing the reflected vector pReflect
		AEVec2Scale(&M, &normal, -AEVec2DotProduct(pVel, &normal) / AEVec2DotProduct(&normal, &normal));
		AEVec2Scale(&M, &M, 2.0f);
		AEVec2Add(&pReflect, pVel, &M);

		//Computing the final position after reflection
		if(pNewPosition)
			AEVec2ScaleAdd(pNewPosition, &pReflect, &collisionPoint, t);

		//Saving the new velocity
		if(pNewVelocity)
			AEVec2Set(pNewVelocity, pReflect.x, pReflect.y);

		return 1;
	}

	return 0;
}
*/
// ---------------------------------------------------------------------------
#endif
