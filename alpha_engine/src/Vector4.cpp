#include "AEExport.h"
#include "AETypes.h"
#include "AEUtil.h"
#include "Vector4.h"

#include <string>
#include <math.h>


// Constructors
Vector4::Vector4(void)
  : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}

Vector4::Vector4(const Vector4& rhs)
  : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
{
}
Vector4::Vector4(f32 xx, f32 yy, f32 zz, f32 ww)
  : x(xx), y(yy), z(zz), w(ww)
{
}

// Assignment operator
Vector4& Vector4::operator=(const Vector4& rhs)
{
  memcpy(v, rhs.v, sizeof(v));
  return (*this);
}

// Unary Operators
Vector4 Vector4::operator-(void) const {
  return Vector4(-x, -y, -z, -w);
}

// Binary Operators
Vector4 Vector4::operator+(const Vector4& rhs) const {
  return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

Vector4 Vector4::operator-(const Vector4& rhs) const {
  return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

Vector4 Vector4::operator*(const f32 rhs) const {
  return Vector4(x * rhs, y * rhs, z * rhs, w * rhs);
}
Vector4 Vector4::operator/(const f32 rhs) const {
  float invRhs = 1.0f / rhs;
  return Vector4(x * invRhs, y * invRhs, z * invRhs, w * invRhs);
}

Vector4& Vector4::operator+=(const Vector4& rhs) {
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
  return (*this);
}

Vector4& Vector4::operator-=(const Vector4& rhs) {
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
  return (*this);
}

Vector4& Vector4::operator*=(const f32 rhs) {
  x *= rhs;
  y *= rhs;
  z *= rhs;
  w *= rhs;
  return (*this);
}

Vector4& Vector4::operator/=(const f32 rhs) {
  float invRhs = 1.0f / rhs;
  x *= invRhs;
  y *= invRhs;
  z *= invRhs;
  w *= invRhs;
  return (*this);
}

bool Vector4::operator==(const Vector4& rhs) const {
  return (AEIsF32Equal(x,rhs.x) && AEIsF32Equal(y,rhs.y) && AEIsF32Equal(z,rhs.z) && AEIsF32Equal(w,rhs.w));
}

bool Vector4::operator!=(const Vector4& rhs) const {
  return !operator==(rhs);
}

// Member functions
f32 Vector4::Dot(const Vector4& rhs) const {
  return (x * rhs.x + y * rhs.y + z * rhs.z);
}
Vector4 Vector4::Cross(const Vector4& rhs) const { 
  return Vector4(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}
f32 Vector4::Length(void) const {
  return sqrtf(x*x + y*y + z*z + w*w);
}
f32 Vector4::LengthSq(void) const {
  return x*x + y*y + z*z + w*w;
}
void Vector4::Normalize(void)
{
  f32 len = Length();

  if(!AEIsF32Zero(len))
  {
    x /= len;
    y /= len;
    z /= len;
    w /= len;
  }
}
void Vector4::Zero(void){
  x = y = z = w = 0.0f;
}
void Vector4::Print(void) const
{
  printf("%5.3f, %5.3f, %5.3f, %5.3f\n",x,y,z,w);
}
