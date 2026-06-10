#include "AEExport.h"
#include "AETypes.h"
#include "AEUtil.h"

#include <string>

#include "Matrix4.h"
#include "Vector4.h"
#include "Point4.h"


// Constructors
Matrix4::Matrix4(void) 
{
  memset(v, 0, sizeof(v));
}

Matrix4::Matrix4(const Matrix4& rhs) 
{
  memcpy(v, rhs.v, sizeof(v));
}

Matrix4::Matrix4(f32 mm00, f32 mm01, f32 mm02, f32 mm03,
                 f32 mm10, f32 mm11, f32 mm12, f32 mm13,
                 f32 mm20, f32 mm21, f32 mm22, f32 mm23,
                 f32 mm30, f32 mm31, f32 mm32, f32 mm33)
  : m00(mm00), m01(mm01), m02(mm02), m03(mm03),
    m10(mm10), m11(mm11), m12(mm12), m13(mm13),
    m20(mm20), m21(mm21), m22(mm22), m23(mm23),
    m30(mm30), m31(mm31), m32(mm32), m33(mm33) 
{
}

Vector4 Matrix4::operator*(const Vector4& rhs) const
{
  return Vector4(
    m00 * rhs.x + m01 * rhs.y + m02 * rhs.z + m03 * rhs.w, 
    m10 * rhs.x + m11 * rhs.y + m12 * rhs.z + m13 * rhs.w, 
    m20 * rhs.x + m21 * rhs.y + m22 * rhs.z + m23 * rhs.w, 
    m30 * rhs.x + m31 * rhs.y + m32 * rhs.z + m33 * rhs.w);
}

Point4 Matrix4::operator*(const Point4& rhs) const
{
  return Point4(
    m00 * rhs.x + m01 * rhs.y + m02 * rhs.z + m03 * rhs.w, 
    m10 * rhs.x + m11 * rhs.y + m12 * rhs.z + m13 * rhs.w, 
    m20 * rhs.x + m21 * rhs.y + m22 * rhs.z + m23 * rhs.w, 
    m30 * rhs.x + m31 * rhs.y + m32 * rhs.z + m33 * rhs.w);
}

Matrix4& Matrix4::operator=(const Matrix4& rhs)
{
  memcpy(v,rhs.v, sizeof(v));
  return (*this);
}

Matrix4 Matrix4::operator+(const Matrix4& rhs) const
{
  return Matrix4(
    m00 + rhs.m00, m01 + rhs.m01, m02 + rhs.m02, m03 + rhs.m03,
    m10 + rhs.m10, m11 + rhs.m11, m12 + rhs.m12, m13 + rhs.m13,
    m20 + rhs.m20, m21 + rhs.m21, m22 + rhs.m22, m23 + rhs.m23,
    m30 + rhs.m30, m31 + rhs.m31, m32 + rhs.m32, m33 + rhs.m33);
}

Matrix4 Matrix4::operator-(const Matrix4& rhs) const
{
  return Matrix4(
    m00 - rhs.m00, m01 - rhs.m01, m02 - rhs.m02, m03 - rhs.m03,
    m10 - rhs.m10, m11 - rhs.m11, m12 - rhs.m12, m13 - rhs.m13,
    m20 - rhs.m20, m21 - rhs.m21, m22 - rhs.m22, m23 - rhs.m23,
    m30 - rhs.m30, m31 - rhs.m31, m32 - rhs.m32, m33 - rhs.m33);
}

Matrix4 Matrix4::operator*(const Matrix4& rhs) const
{
  return Matrix4(
    m00 * rhs.m00 + m01 * rhs.m10 + m02 * rhs.m20 + m03 * rhs.m30, 
    m00 * rhs.m01 + m01 * rhs.m11 + m02 * rhs.m21 + m03 * rhs.m31, 
    m00 * rhs.m02 + m01 * rhs.m12 + m02 * rhs.m22 + m03 * rhs.m32, 
    m00 * rhs.m03 + m01 * rhs.m13 + m02 * rhs.m23 + m03 * rhs.m33, 

    m10 * rhs.m00 + m11 * rhs.m10 + m12 * rhs.m20 + m13 * rhs.m30,
    m10 * rhs.m01 + m11 * rhs.m11 + m12 * rhs.m21 + m13 * rhs.m31, 
    m10 * rhs.m02 + m11 * rhs.m12 + m12 * rhs.m22 + m13 * rhs.m32,
    m10 * rhs.m03 + m11 * rhs.m13 + m12 * rhs.m23 + m13 * rhs.m33, 

    m20 * rhs.m00 + m21 * rhs.m10 + m22 * rhs.m20 + m23 * rhs.m30,
    m20 * rhs.m01 + m21 * rhs.m11 + m22 * rhs.m21 + m23 * rhs.m31, 
    m20 * rhs.m02 + m21 * rhs.m12 + m22 * rhs.m22 + m23 * rhs.m32, 
    m20 * rhs.m03 + m21 * rhs.m13 + m22 * rhs.m23 + m23 * rhs.m33, 

    m30 * rhs.m00 + m31 * rhs.m10 + m32 * rhs.m20 + m33 * rhs.m30,
    m30 * rhs.m01 + m31 * rhs.m11 + m32 * rhs.m21 + m33 * rhs.m31, 
    m30 * rhs.m02 + m31 * rhs.m12 + m32 * rhs.m22 + m33 * rhs.m32, 
    m30 * rhs.m03 + m31 * rhs.m13 + m32 * rhs.m23 + m33 * rhs.m33);
}

Matrix4& Matrix4::operator+=(const Matrix4& rhs) {
  return (*this) = (*this) + rhs;
}

Matrix4& Matrix4::operator-=(const Matrix4& rhs) {
  return (*this) = (*this) - rhs;
}

Matrix4& Matrix4::operator*=(const Matrix4& rhs) {
  return (*this) = (*this) * rhs;
}

Matrix4 Matrix4::operator*(const f32 rhs) const
{
  return Matrix4(
    m00 * rhs, m01 * rhs, m02 * rhs, m03 * rhs,
    m10 * rhs, m11 * rhs, m12 * rhs, m13 * rhs,
    m20 * rhs, m21 * rhs, m22 * rhs, m23 * rhs,
    m30 * rhs, m31 * rhs, m32 * rhs, m33 * rhs);
}

Matrix4 Matrix4::operator/(const f32 rhs) const
{
  float invRhs = 1.0f / rhs;
  return Matrix4(
    m00 * invRhs, m01 * invRhs, m02 * invRhs, m03 * invRhs,
    m10 * invRhs, m11 * invRhs, m12 * invRhs, m13 * invRhs,
    m20 * invRhs, m21 * invRhs, m22 * invRhs, m23 * invRhs,
    m30 * invRhs, m31 * invRhs, m32 * invRhs, m33 * invRhs);
}

Matrix4& Matrix4::operator*=(const f32 rhs) {
  return (*this) = (*this) * rhs;
}

Matrix4& Matrix4::operator/=(const f32 rhs) {
  return (*this) = (*this) * (1.0f / rhs);
}

bool Matrix4::operator==(const Matrix4& rhs) const
{
  return AEIsF32Equal(v[0],  rhs.v[0])  && AEIsF32Equal(v[1],  rhs.v[1])  && AEIsF32Equal(v[2],  rhs.v[2])  && AEIsF32Equal(v[3],  rhs.v[3])  && 
         AEIsF32Equal(v[4],  rhs.v[4])  && AEIsF32Equal(v[5],  rhs.v[5])  && AEIsF32Equal(v[6],  rhs.v[6])  && AEIsF32Equal(v[7],  rhs.v[7])  &&
         AEIsF32Equal(v[8],  rhs.v[8])  && AEIsF32Equal(v[9],  rhs.v[9])  && AEIsF32Equal(v[10], rhs.v[10]) && AEIsF32Equal(v[11], rhs.v[11]) &&
         AEIsF32Equal(v[12], rhs.v[12]) && AEIsF32Equal(v[13], rhs.v[13]) && AEIsF32Equal(v[14], rhs.v[14]) && AEIsF32Equal(v[15], rhs.v[15]);
}
bool Matrix4::operator!=(const Matrix4& rhs) const
{
  return !(operator==(rhs));
}

void Matrix4::Zero(void)
{
  m00 = m01 = m02 = m03 =
  m10 = m11 = m12 = m13 =
  m20 = m21 = m22 = m23 =
  m30 = m31 = m32 = m33 = 0.0f;
}

void Matrix4::Identity(void)
{
  m01 = m02 = m03 = m10 = 
  m12 = m13 = m20 = m21 = 
  m23 = m30 = m31 = m32 = 0.0f;

  m00 = m11 = m22 = m33 = 1.0f;
}

void Matrix4::Print(void) const
{
  printf("--------------------------\n");
  printf("%5.3f %5.3f %5.3f %5.3f\n", m00, m01, m02, m03 );
  printf("%5.3f %5.3f %5.3f %5.3f\n", m10, m11, m12, m13 );
  printf("%5.3f %5.3f %5.3f %5.3f\n", m20, m21, m22, m23 );
  printf("%5.3f %5.3f %5.3f %5.3f\n", m30, m31, m32, m33 );
  printf("--------------------------\n");
}
