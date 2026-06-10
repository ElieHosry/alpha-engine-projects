// ---------------------------------------------------------------------------
// Project Name		:	Alpha Engine
// File Name		:	AEMtx33.c
// Author			:	Sun Tjen Fam
// Creation Date	:	2008/01/31
// Purpose			:	implementation of the 3x3 matrix library
// History			:
// - 2008/01/31		:	- initial implementation
// ---------------------------------------------------------------------------

#include "AEExport.h"
#include "AETypes.h"
#include "AEVec2.h"
#include "AEMtx33.h"
//#include "AEMath.h"
#include <math.h>


// ---------------------------------------------------------------------------


AE_API void AEMtx33Identity(AEMtx33* pResult)
{
	AEMtx33RowCol(pResult, 0, 0) = 
	AEMtx33RowCol(pResult, 1, 1) = 
	AEMtx33RowCol(pResult, 2, 2) = 1.0f;

	AEMtx33RowCol(pResult, 0, 1) = 
	AEMtx33RowCol(pResult, 0, 2) = 
	AEMtx33RowCol(pResult, 1, 0) = 
	AEMtx33RowCol(pResult, 1, 2) = 
	AEMtx33RowCol(pResult, 2, 0) = 
	AEMtx33RowCol(pResult, 2, 1) = 0.0f;
}

//DECLARE_FUNCTION(AEMtx33Identity, AEMtx33*);

// ---------------------------------------------------------------------------

AE_API void AEMtx33Transpose(AEMtx33* pResult, const AEMtx33* pMtx)
{
	if (pResult == pMtx)
	{
		for (u32 i = 1; i < 3; i++)
		{
			for (u32 j = 0; j < i; j++)
			{
				f32 f						 = AEMtx33RowCol(pResult, i, j);
				AEMtx33RowCol(pResult, i, j) = AEMtx33RowCol(pResult, j, i);
				AEMtx33RowCol(pResult, j, i) = f;
			}
		}
	}
	else
	{
		for (u32 i = 0; i < 3; i++)
			for (u32 j = 0; j < 3; j++)
				AEMtx33RowCol(pResult, i, j) = AEMtx33RowCol(pMtx, j, i);
	}
}


// ---------------------------------------------------------------------------


AE_API f32 AEMtx33Inverse(AEMtx33* pResult, const AEMtx33* pMtx)
{
	AEMtx33 m;
	f32     d;

	// calculate the adjoint matrix of the given matrix
	AEMtx33RowCol(&m, 0, 0) =	AEMtx33RowCol(pMtx, 1, 1) * AEMtx33RowCol(pMtx, 2, 2) - AEMtx33RowCol(pMtx, 1, 2) * AEMtx33RowCol(pMtx, 2, 1);
	AEMtx33RowCol(&m, 1, 0) =	AEMtx33RowCol(pMtx, 1, 2) * AEMtx33RowCol(pMtx, 2, 0) - AEMtx33RowCol(pMtx, 1, 0) * AEMtx33RowCol(pMtx, 2, 2);
	AEMtx33RowCol(&m, 2, 0) =	AEMtx33RowCol(pMtx, 1, 0) * AEMtx33RowCol(pMtx, 2, 1) - AEMtx33RowCol(pMtx, 1, 1) * AEMtx33RowCol(pMtx, 2, 0);

	// calculate the determinant
	d					 =	AEMtx33RowCol(pMtx, 0, 0) * AEMtx33RowCol(&m, 0, 0) + 
							AEMtx33RowCol(pMtx, 0, 1) * AEMtx33RowCol(&m, 1, 0) +
							AEMtx33RowCol(pMtx, 0, 2) * AEMtx33RowCol(&m, 2, 0);

	// if determinant is small, return 0.0f
	if ((d * d) < (EPSILON * EPSILON))
		return 0.0f;

	AEMtx33RowCol(&m, 0, 1) =	AEMtx33RowCol(pMtx, 0, 2) * AEMtx33RowCol(pMtx, 2, 1) - AEMtx33RowCol(pMtx, 0, 1) * AEMtx33RowCol(pMtx, 2, 2);
	AEMtx33RowCol(&m, 1, 1) =	AEMtx33RowCol(pMtx, 0, 0) * AEMtx33RowCol(pMtx, 2, 2) - AEMtx33RowCol(pMtx, 0, 2) * AEMtx33RowCol(pMtx, 2, 0);
	AEMtx33RowCol(&m, 2, 1) =	AEMtx33RowCol(pMtx, 0, 1) * AEMtx33RowCol(pMtx, 2, 0) - AEMtx33RowCol(pMtx, 0, 0) * AEMtx33RowCol(pMtx, 2, 1);

	AEMtx33RowCol(&m, 0, 2) =	AEMtx33RowCol(pMtx, 0, 1) * AEMtx33RowCol(pMtx, 1, 2) - AEMtx33RowCol(pMtx, 0, 2) * AEMtx33RowCol(pMtx, 1, 1);
	AEMtx33RowCol(&m, 1, 2) =	AEMtx33RowCol(pMtx, 0, 2) * AEMtx33RowCol(pMtx, 1, 0) - AEMtx33RowCol(pMtx, 0, 0) * AEMtx33RowCol(pMtx, 1, 2);
	AEMtx33RowCol(&m, 2, 2) =	AEMtx33RowCol(pMtx, 0, 0) * AEMtx33RowCol(pMtx, 1, 1) - AEMtx33RowCol(pMtx, 0, 1) * AEMtx33RowCol(pMtx, 1, 0);

	// the inverse = adjoint matrix / determinant
	AEMtx33RowCol(pResult, 0, 0) = AEMtx33RowCol(&m, 0, 0) / d;
	AEMtx33RowCol(pResult, 0, 1) = AEMtx33RowCol(&m, 0, 1) / d;
	AEMtx33RowCol(pResult, 0, 2) = AEMtx33RowCol(&m, 0, 2) / d;

	AEMtx33RowCol(pResult, 1, 0) = AEMtx33RowCol(&m, 1, 0) / d;
	AEMtx33RowCol(pResult, 1, 1) = AEMtx33RowCol(&m, 1, 1) / d;
	AEMtx33RowCol(pResult, 1, 2) = AEMtx33RowCol(&m, 1, 2) / d;

	AEMtx33RowCol(pResult, 2, 0) = AEMtx33RowCol(&m, 2, 0) / d;
	AEMtx33RowCol(pResult, 2, 1) = AEMtx33RowCol(&m, 2, 1) / d;
	AEMtx33RowCol(pResult, 2, 2) = AEMtx33RowCol(&m, 2, 2) / d;

	return d;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33InvTranspose(AEMtx33* pResult, const AEMtx33* pMtx)
{
	AEMtx33Inverse	(pResult, pMtx);
	AEMtx33Transpose(pResult, pResult);
}


// ---------------------------------------------------------------------------

AE_API void AEMtx33Concat(AEMtx33* pResult, const AEMtx33* pMtx0, const AEMtx33* pMtx1)
{
	u32			i, j;
	AEMtx33		temp;
	AEMtx33*	pTemp;

	// if the result pointer is one of the source pointer, use temporary matrix
	pTemp = ((pResult == pMtx0) || (pResult == pMtx1)) ? &temp : pResult;

	// multiply the 2 given matrices
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			AEMtx33RowCol(pTemp, i, j) = 
				AEMtx33RowCol(pMtx0, i, 0) * AEMtx33RowCol(pMtx1, 0, j) + 
				AEMtx33RowCol(pMtx0, i, 1) * AEMtx33RowCol(pMtx1, 1, j) + 
				AEMtx33RowCol(pMtx0, i, 2) * AEMtx33RowCol(pMtx1, 2, j);

	// if using temporary buffer, copy the matrix
	if (pTemp != pResult)
		*pResult = *pTemp;
}


// ---------------------------------------------------------------------------

AE_API void AEMtx33Orthogonalize(AEMtx33* pResult, const AEMtx33* pMtx)
{
	pResult = 0;
	pMtx = 0;
//AE_FATAL_ERROR("Not implemented yet!!");
}

// ---------------------------------------------------------------------------

AE_API f32 AEMtx33Determinant(const AEMtx33* pMtx)
{
	return 
		AEMtx33RowCol(pMtx, 0, 0) * (AEMtx33RowCol(pMtx, 1, 1) * AEMtx33RowCol(pMtx, 2, 2) - AEMtx33RowCol(pMtx, 1, 2) * AEMtx33RowCol(pMtx, 2, 1)) - 
		AEMtx33RowCol(pMtx, 0, 1) * (AEMtx33RowCol(pMtx, 1, 0) * AEMtx33RowCol(pMtx, 2, 2) - AEMtx33RowCol(pMtx, 1, 2) * AEMtx33RowCol(pMtx, 2, 0)) + 
		AEMtx33RowCol(pMtx, 0, 2) * (AEMtx33RowCol(pMtx, 1, 0) * AEMtx33RowCol(pMtx, 2, 1) - AEMtx33RowCol(pMtx, 1, 1) * AEMtx33RowCol(pMtx, 2, 0));
}



// ---------------------------------------------------------------------------
// 'access' functions

AE_API void AEMtx33SetCol(AEMtx33* pResult, u32 col, const AEVec2* pVec)
{
	AEMtx33RowCol(pResult, 0, col) = pVec->x;
	AEMtx33RowCol(pResult, 1, col) = pVec->y;
	AEMtx33RowCol(pResult, 2, col) = (col == 2) ? 1.0f : 0.0f;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33SetRow(AEMtx33* pResult, u32 row, const AEVec2* pVec)
{
	AEMtx33RowCol(pResult, row, 0) = pVec->x;
	AEMtx33RowCol(pResult, row, 1) = pVec->y;
	AEMtx33RowCol(pResult, row, 2) = (row == 2) ? 1.0f : 0.0f;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33GetCol(AEVec2* pResult, const AEMtx33* pMtx, u32 col)
{
	pResult->x = AEMtx33RowCol(pMtx, 0, col);
	pResult->y = AEMtx33RowCol(pMtx, 1, col);
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33GetRow(AEVec2* pResult, const AEMtx33* pMtx, u32 row)
{
	pResult->x = AEMtx33RowCol(pMtx, row, 0);
	pResult->y = AEMtx33RowCol(pMtx, row, 1);
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33Trans(AEMtx33* pResult, f32 x, f32 y)
{
	AEMtx33RowCol(pResult, 0, 0) = 
	AEMtx33RowCol(pResult, 1, 1) = 
	AEMtx33RowCol(pResult, 2, 2) = 1.0f;

	AEMtx33RowCol(pResult, 0, 1) = 0.0f;
	AEMtx33RowCol(pResult, 0, 2) = x;

	AEMtx33RowCol(pResult, 1, 0) = 0.0f;
	AEMtx33RowCol(pResult, 1, 2) = y;

	AEMtx33RowCol(pResult, 2, 0) = 
	AEMtx33RowCol(pResult, 2, 1) = 0.0f;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33TransApply(AEMtx33* pResult, const AEMtx33* pMtx, f32 x, f32 y)
{
	if (pResult != pMtx)
		*pResult = *pMtx;

	AEMtx33RowCol(pResult, 0, 2) += x;
	AEMtx33RowCol(pResult, 1, 2) += y;
}


// ---------------------------------------------------------------------------

AE_API void AEMtx33Scale(AEMtx33* pResult, f32 x, f32 y)
{
	AEMtx33RowCol(pResult, 0, 0) = x;
	AEMtx33RowCol(pResult, 1, 1) = y;
	AEMtx33RowCol(pResult, 2, 2) = 1.0f;

	AEMtx33RowCol(pResult, 0, 1) = 
	AEMtx33RowCol(pResult, 0, 2) = 0.0f;

	AEMtx33RowCol(pResult, 1, 0) = 
	AEMtx33RowCol(pResult, 1, 2) = 0.0f;

	AEMtx33RowCol(pResult, 2, 0) = 
	AEMtx33RowCol(pResult, 2, 1) = 0.0f;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33ScaleApply(AEMtx33* pResult, const AEMtx33* pMtx, f32 x, f32 y)
{
	AEMtx33RowCol(pResult, 0, 0) = AEMtx33RowCol(pMtx, 0, 0) * x;
	AEMtx33RowCol(pResult, 0, 1) = AEMtx33RowCol(pMtx, 0, 1) * x;
	AEMtx33RowCol(pResult, 0, 2) = AEMtx33RowCol(pMtx, 0, 2) * x;

	AEMtx33RowCol(pResult, 1, 0) = AEMtx33RowCol(pMtx, 1, 0) * y;
	AEMtx33RowCol(pResult, 1, 1) = AEMtx33RowCol(pMtx, 1, 1) * y;
	AEMtx33RowCol(pResult, 1, 2) = AEMtx33RowCol(pMtx, 1, 2) * y;

	if (pResult != pMtx)
	{
		AEMtx33RowCol(pResult, 2, 0) = AEMtx33RowCol(pMtx, 2, 0);
		AEMtx33RowCol(pResult, 2, 1) = AEMtx33RowCol(pMtx, 2, 1);
		AEMtx33RowCol(pResult, 2, 2) = AEMtx33RowCol(pMtx, 2, 2);
	}
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33Rot(AEMtx33* pResult, f32 angle)
{
	f32 s = sinf(angle), 
		c = cosf(angle);
	
	AEMtx33RowCol(pResult, 0, 0) = c;
	AEMtx33RowCol(pResult, 0, 1) = -s;
	AEMtx33RowCol(pResult, 0, 2) = 0.0f;
	AEMtx33RowCol(pResult, 1, 0) = s;
	AEMtx33RowCol(pResult, 1, 1) = c;
	AEMtx33RowCol(pResult, 1, 2) = 0.0f;
	AEMtx33RowCol(pResult, 2, 0) = 
	AEMtx33RowCol(pResult, 2, 1) = 0.0f;
	AEMtx33RowCol(pResult, 2, 2) = 1.0f;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33RotDeg(AEMtx33* pResult, f32 angle)
{
	AEMtx33Rot(pResult, angle * 3.1415f / 180.0f);//AEDegToRad(angle));
}


// ---------------------------------------------------------------------------

AE_API void AEMtx33MultVec(AEVec2* pResult, const AEMtx33* pMtx, const AEVec2* pVec)
{
	AEVec2 tmpVec;

	tmpVec.x = AEMtx33RowCol(pMtx, 0, 0) * pVec->x + AEMtx33RowCol(pMtx, 0, 1) * pVec->y + AEMtx33RowCol(pMtx, 0, 2);
	tmpVec.y = AEMtx33RowCol(pMtx, 1, 0) * pVec->x + AEMtx33RowCol(pMtx, 1, 1) * pVec->y + AEMtx33RowCol(pMtx, 1, 2);
	
	*pResult = tmpVec;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33MultVecArray(AEVec2* pResult, const AEMtx33* pMtx, const AEVec2* pVec, u32 count)
{
	AEVec2 tmpVec;

	for (u32 i = 0; i < count; i++)
	{
		tmpVec.x = AEMtx33RowCol(pMtx, 0, 0) * pVec[i].x + AEMtx33RowCol(pMtx, 0, 1) * pVec[i].y + AEMtx33RowCol(pMtx, 0, 2);
		tmpVec.y = AEMtx33RowCol(pMtx, 1, 0) * pVec[i].x + AEMtx33RowCol(pMtx, 1, 1) * pVec[i].y + AEMtx33RowCol(pMtx, 1, 2);

		pResult[i] = tmpVec;
	}
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33MultVecSR(AEVec2* pResult, const AEMtx33* pMtx, const AEVec2* pVec)
{
	AEVec2 tmpVec;

	tmpVec.x = AEMtx33RowCol(pMtx, 0, 0) * pVec->x + AEMtx33RowCol(pMtx, 0, 1) * pVec->y;
	tmpVec.y = AEMtx33RowCol(pMtx, 1, 0) * pVec->x + AEMtx33RowCol(pMtx, 1, 1) * pVec->y;

	*pResult = tmpVec;
}


// ---------------------------------------------------------------------------


AE_API void AEMtx33MultVecArraySR(AEVec2* pResult, const AEMtx33* pMtx, const AEVec2* pVec, u32 count)
{
	AEVec2 tmpVec;

	for (u32 i = 0; i < count; i++)
	{
		tmpVec.x = AEMtx33RowCol(pMtx, 0, 0) * pVec[i].x + AEMtx33RowCol(pMtx, 0, 1) * pVec[i].y;
		tmpVec.y = AEMtx33RowCol(pMtx, 1, 0) * pVec[i].x + AEMtx33RowCol(pMtx, 1, 1) * pVec[i].y;

		pResult[i] = tmpVec;
	}
}


// ---------------------------------------------------------------------------


