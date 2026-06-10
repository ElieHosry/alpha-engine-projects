// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEVec2.cpp
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/01/31
// Purpose			:	implementation of the 2D vector library
// History			:
// - 2008/01/31		:	- initial implementation
// ---------------------------------------------------------------------------

#include <math.h>
#include "AEVec2.h"
//#include "AEMath.h"

// ---------------------------------------------------------------------------


extern "C" {

AE_API s32 AEVec2Test(s32 i1, s32 i2)
{
	return i1+i2;
}


/*
JNIEXPORT s32 Java_com_example_alphaengine_MainActivity_AEVec2Test(JNIEnv * env, jobject obj, s32 i1, s32 i2)
{
	LOGI("Adding %i & %i", i1, i2);
	return i1+i2;
}*/
}


AE_API void AEVec2Zero(AEVec2* pResult)
{
	pResult->x = pResult->y = 0.0f;
}



// ---------------------------------------------------------------------------


AE_API void AEVec2Set(AEVec2* pResult, f32 x, f32 y)
{
	pResult->x = x;
	pResult->y = y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Neg(AEVec2* pResult, AEVec2* pVec0)
{
	pResult->x = -pVec0->x;
	pResult->y = -pVec0->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Add(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1)
{
	pResult->x = pVec0->x + pVec1->x;
	pResult->y = pVec0->y + pVec1->y;
}



// ---------------------------------------------------------------------------


AE_API void AEVec2Sub(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1)
{
	pResult->x = pVec0->x - pVec1->x;
	pResult->y = pVec0->y - pVec1->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Normalize(AEVec2* pResult, AEVec2* pVec0)
{
	f32 length = 1.0f / AEVec2Length(pVec0);

	pResult->x = length * pVec0->x;
	pResult->y = length * pVec0->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Scale(AEVec2* pResult, AEVec2* pVec0, f32 s)
{
	pResult->x = s * pVec0->x;
	pResult->y = s * pVec0->y;
}



// ---------------------------------------------------------------------------


AE_API void AEVec2ScaleAdd(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1, f32 s)
{
	pResult->x = s * pVec0->x + pVec1->x;
	pResult->y = s * pVec0->y + pVec1->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2ScaleSub(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1, f32 s)
{
	pResult->x = s * pVec0->x - pVec1->x;
	pResult->y = s * pVec0->y - pVec1->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Project(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1)
{
	AEVec2 u;

	AEVec2Normalize(&u, pVec1);
	AEVec2Scale	 (pResult, &u, AEVec2DotProduct(pVec0, &u));
}


// ---------------------------------------------------------------------------


AE_API void AEVec2ProjectPerp(AEVec2* pResult, AEVec2* pVec0, AEVec2* pVec1)
{
	AEVec2Project(pResult, pVec0, pVec1);
	AEVec2Sub    (pResult, pVec0, pResult);
}


// ---------------------------------------------------------------------------


AE_API void AEVec2Lerp(AEVec2* pResult, AEVec2 *pVec0, AEVec2 *pVec1, f32 t)
{
	pResult->x = pVec0->x + (pVec1->x - pVec0->x) * t;
	pResult->y = pVec0->y + (pVec1->y - pVec0->y) * t;
}


// ---------------------------------------------------------------------------


AE_API f32	AEVec2Length(AEVec2* pVec0)
{
	return sqrtf(pVec0->x * pVec0->x + pVec0->y * pVec0->y);
}


// ---------------------------------------------------------------------------


AE_API f32 AEVec2SquareLength(AEVec2* pVec0)
{
	return pVec0->x * pVec0->x + pVec0->y * pVec0->y;
}


// ---------------------------------------------------------------------------


AE_API f32 AEVec2Distance(AEVec2* pVec0, AEVec2* pVec1)
{
	AEVec2 d;

	d.x = pVec0->x - pVec1->x; 
	d.y = pVec0->y - pVec1->y;

	return sqrtf(d.x * d.x + d.y * d.y);
}


// ---------------------------------------------------------------------------


AE_API f32 AEVec2SquareDistance(AEVec2* pVec0, AEVec2* pVec1)
{
	AEVec2 d;

	d.x = pVec0->x - pVec1->x; 
	d.y = pVec0->y - pVec1->y;

	return (d.x * d.x + d.y * d.y);
}


// ---------------------------------------------------------------------------


AE_API f32 AEVec2DotProduct(AEVec2* pVec0, AEVec2* pVec1)
{
	return pVec0->x * pVec1->x + pVec0->y * pVec1->y;
}


// ---------------------------------------------------------------------------


AE_API f32 AEVec2CrossProductMag(AEVec2* pVec0, AEVec2* pVec1)
{
	return pVec0->x * pVec1->y - pVec1->x * pVec0->y;
}


// ---------------------------------------------------------------------------


AE_API void AEVec2FromAngle(AEVec2* pResult, f32 angle)
{
	pResult->x = cosf(angle);
	pResult->y = sinf(angle);
}


// ---------------------------------------------------------------------------
