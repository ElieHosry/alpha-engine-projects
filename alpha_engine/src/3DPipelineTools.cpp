#include "AEExport.h"
#include "AETypes.h"
#include "AEUtil.h"

#include "Matrix4.h"
#include "Vector4.h"
#include "Point4.h"
#include "3DPipelineTools.h"

#include <math.h>

/////////////////////////////////////////////////////////////////
Matrix4 MtxScale(const f32 &sx, const f32 &sy, const f32 &sz)
{
	Matrix4 m;

	m.m[0][0] = sx;			m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f;		m.m[1][1] = sy;			m.m[1][2] = 0.0f;		m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = sz;			m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxScale(const Point4 &s)
{
	Matrix4 m;

	m.m[0][0] = s.x;		m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f;		m.m[1][1] = s.y;		m.m[1][2] = 0.0f;		m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = s.z;		m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxScale(const Vector4 &s)
{
	Matrix4 m;

	m.m[0][0] = s.x;		m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f;		m.m[1][1] = s.y;		m.m[1][2] = 0.0f;		m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = s.z;		m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxTranslate(const f32 &tx, const f32 &ty, const f32 &tz)
{
	Matrix4 m;

	m.m[0][0] = 1.0f;		m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = tx;
	m.m[1][0] = 0.0f;		m.m[1][1] = 1.0f;		m.m[1][2] = 0.0f;		m.m[1][3] = ty;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = 1.0f;		m.m[2][3] = tz;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxTranslate(const Point4 &t)
{
	Matrix4 m;

	m.m[0][0] = 1.0f;		m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = t.x;
	m.m[1][0] = 0.0f;		m.m[1][1] = 1.0f;		m.m[1][2] = 0.0f;		m.m[1][3] = t.y;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = 1.0f;		m.m[2][3] = t.z;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxTranslate(const Vector4 &t)
{
	Matrix4 m;

	m.m[0][0] = 1.0f;		m.m[0][1] = 0.0f;		m.m[0][2] = 0.0f;		m.m[0][3] = t.x;
	m.m[1][0] = 0.0f;		m.m[1][1] = 1.0f;		m.m[1][2] = 0.0f;		m.m[1][3] = t.y;
	m.m[2][0] = 0.0f;		m.m[2][1] = 0.0f;		m.m[2][2] = 1.0f;		m.m[2][3] = t.z;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxRotateX(const f32 &alpha)
{
	Matrix4 m;

	m.m[0][0] = 1.0f;		m.m[0][1] = 0.0f;				m.m[0][2] = 0.0f;				m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f;		m.m[1][1] = cosf(alpha);		m.m[1][2] = -sinf(alpha);		m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f;		m.m[2][1] = sinf(alpha);		m.m[2][2] = cosf(alpha);		m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;		m.m[3][1] = 0.0f;				m.m[3][2] = 0.0f;				m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxRotateY(const f32 &alpha)
{
	Matrix4 m;

	m.m[0][0] = cosf(alpha);		m.m[0][1] = 0.0f;		m.m[0][2] = sinf(alpha);		m.m[0][3] = 0.0f;
	m.m[1][0] = 0.0f;				m.m[1][1] = 1.0f;		m.m[1][2] = 0.0f;				m.m[1][3] = 0.0f;
	m.m[2][0] = -sinf(alpha);		m.m[2][1] = 0.0f;		m.m[2][2] = cosf(alpha);		m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;				m.m[3][1] = 0.0f;		m.m[3][2] = 0.0f;				m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxRotateZ(const f32 &alpha)
{
	Matrix4 m;

	m.m[0][0] = cosf(alpha);		m.m[0][1] = -sinf(alpha);		m.m[0][2] = 0.0f;		m.m[0][3] = 0.0f;
	m.m[1][0] = sinf(alpha);		m.m[1][1] = cosf(alpha);		m.m[1][2] = 0.0f;		m.m[1][3] = 0.0f;
	m.m[2][0] = 0.0f;				m.m[2][1] = 0.0f;				m.m[2][2] = 1.0f;		m.m[2][3] = 0.0f;
	m.m[3][0] = 0.0f;				m.m[3][1] = 0.0f;				m.m[3][2] = 0.0f;		m.m[3][3] = 1.0f;

	return m;
}

Matrix4 MtxRotateAxisAngle(Vector4 &v, const f32 &alpha)
{
	Matrix4 result;
	result.Identity();

	if (AEIsF32Zero(v.Length()))
		return result;

	v.Normalize();

	f32 c = cosf(alpha);
	f32 s = sinf(alpha);
	f32 cc = 1 - c;

	result.m[0][0] = c + cc * v.x * v.x;			result.m[0][1] = -s * v.z + cc * v.x * v.y;			result.m[0][2] = s * v.y + cc * v.x * v.z;
	result.m[1][0] = s * v.z + cc * v.x * v.y;		result.m[1][1] = c + cc * v.y * v.y;				result.m[1][2] = -s * v.x + cc * v.y * v.z;
	result.m[2][0] = -s * v.y + cc * v.x * v.z;		result.m[2][1] = s * v.x + cc * v.y * v.z;			result.m[2][2] = c + cc * v.z * v.z;

	return result;
}

Matrix4 MtxRotateOrthogonal(Vector4 &u, Vector4 &v, Vector4 &w)
{
	Matrix4 result;
	result.Identity();

	u.Normalize();
	v.Normalize();
	w.Normalize();

	result.m[0][0] = u.x;			result.m[0][1] = v.x;			result.m[0][2] = w.x;
	result.m[1][0] = u.y;			result.m[1][1] = v.y;			result.m[1][2] = w.y;
	result.m[2][0] = u.z;			result.m[2][1] = v.z;			result.m[2][2] = w.z;

	return result;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxTranspose(const Matrix4 &m)
{
	Matrix4 result;

	for (u32 i=0; i<4; ++i)
		for (u32 j=0; j<4; ++j)
			result.m[i][j] = m.m[j][i];

	return result;
}

f32 MtxDeterminant(const Matrix4 &m)
{
	f32 result = 0.0f;

	result += m.m[0][0] * (m.m[1][1] * (m.m[2][2] * m.m[3][3] - m.m[3][2] * m.m[2][3]) - m.m[1][2] * (m.m[2][1] * m.m[3][3] - m.m[3][1] * m.m[2][3]) + m.m[1][3] * (m.m[2][1] * m.m[3][2] - m.m[3][1] * m.m[2][2]));
	result -= m.m[1][0] * (m.m[1][0] * (m.m[2][2] * m.m[3][3] - m.m[3][2] * m.m[2][3]) - m.m[1][2] * (m.m[2][0] * m.m[3][3] - m.m[3][0] * m.m[2][3]) + m.m[1][3] * (m.m[2][0] * m.m[3][2] - m.m[3][0] * m.m[2][2]));
	result += m.m[2][0] * (m.m[1][0] * (m.m[2][1] * m.m[3][3] - m.m[3][1] * m.m[2][3]) - m.m[1][1] * (m.m[2][0] * m.m[3][3] - m.m[3][0] * m.m[2][3]) + m.m[1][3] * (m.m[2][0] * m.m[3][1] - m.m[3][0] * m.m[2][1]));
	result -= m.m[3][0] * (m.m[1][0] * (m.m[2][1] * m.m[3][2] - m.m[3][1] * m.m[2][2]) - m.m[1][1] * (m.m[2][0] * m.m[3][2] - m.m[3][0] * m.m[2][2]) + m.m[1][2] * (m.m[2][0] * m.m[3][1] - m.m[3][0] * m.m[2][1]));

	return result;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxLookAt(const Point4 &position, const Point4 &target, const Vector4 &up)
{
    Matrix4 result;
    
    Vector4 direction = position - target;
    direction.Normalize();
    
    Vector4 u = up;
    u.Normalize();
    
	Vector4 r = u.Cross(direction);
    r.Normalize();
    
    result = MtxTranspose(MtxRotateOrthogonal(r, u, direction)) * MtxTranslate(-position);
    
    return result;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxOrthogonalProjection(const f32 &width, const f32 &height, const f32 &near, const f32 &far)
{
    Matrix4 result;
    result.Identity();

    f32 depth = far - near;
    
    result.m[0][0] = 2.0f / width;
    result.m[1][1] = 2.0f / height;
    result.m[2][2] = -2.0f / depth;
    
    result.m[2][3] = -(far + near) / depth;
    
    return result;
}

Matrix4 MtxPerspectiveProjection(const f32 &fovy, const f32 &aspectRatio, const f32 &near, const f32 &far)
{
	Matrix4 result;
	result.Identity();
    
    f32 f = tanf(fovy / 360.0f * PI);
    
	result.m[0][0] = 1.0f / (f * aspectRatio);
	result.m[1][1] = 1.0f / f;
	result.m[2][2] = -(near + far) / (far - near);
	result.m[2][3] = -(2.0f * near * far) / (far - near);
	result.m[3][2] = -1.0f;
	result.m[3][3] = 0.0f;
    
	return result;
}
/////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
Matrix4 MtxNormalMatrix(const Matrix4 &input)
{
    Matrix4 result;
    
    f32 tx = input.m03, ty = input.m13, tz = input.m23;
    f32 sx = input.m00 * input.m00 + input.m10 * input.m10 + input.m20 * input.m20;
    f32 sy = input.m01 * input.m01 + input.m11 * input.m11 + input.m21 * input.m21;
    f32 sz = input.m02 * input.m02 + input.m12 * input.m12 + input.m22 * input.m22;
    
    result.m00 = input.m00 / sx;    result.m01 = input.m01 / sy;    result.m02 = input.m02 / sz;    result.m03 = 0.0f;
    result.m10 = input.m10 / sx;    result.m11 = input.m21 / sy;    result.m12 = input.m12 / sz;    result.m13 = 0.0f;
    result.m20 = input.m20 / sx;    result.m21 = input.m21 / sy;    result.m22 = input.m22 / sz;    result.m23 = 0.0f;
    
    result.m30 = -tx * result.m00 - ty * result.m10 - tz * result.m20;
    result.m31 = -tx * result.m01 - ty * result.m11 - tz * result.m21;
    result.m32 = -tx * result.m02 - ty * result.m12 - tz * result.m22;
    
    result.m33 = 1.0f;
    
    return result;
}
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////