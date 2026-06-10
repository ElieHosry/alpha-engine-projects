#include "AEExport.h"
#include "AETypes.h"
#include "AEUtil.h"
#include "Vector4.h"
#include "Point4.h"
#include <string>

Point4::Point4(void)
  : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}
Point4::Point4(const Point4& rhs)
  : x(rhs.x), y(rhs.y), z(rhs.z), w(rhs.w)
{
}
Point4::Point4(f32 xx, f32 yy, f32 zz, f32 ww)
  : x(xx), y(yy), z(zz), w(ww)
{
}

Point4& Point4::operator=(const Point4& rhs)
{
  memcpy(v, rhs.v, sizeof(v));
  return (*this);
}

Point4 Point4::operator-(void) const {
  return Point4(-x,-y,-z,-w);
}

Vector4 Point4::operator-(const Point4& rhs) const {
  return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

Point4  Point4::operator+(const Vector4& rhs) const {
  return Point4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}
Point4  Point4::operator-(const Vector4& rhs) const {
  return Point4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}
Point4& Point4::operator+=(const Vector4& rhs)
{
  x += rhs.x;
  y += rhs.y;
  z += rhs.z;
  w += rhs.w;
  return (*this);
}
Point4& Point4::operator-=(const Vector4& rhs)
{
  x -= rhs.x;
  y -= rhs.y;
  z -= rhs.z;
  w -= rhs.w;
  return (*this);
}

bool Point4::operator==(const Point4& rhs) const {
  return (AEIsF32Equal(x,rhs.x) && AEIsF32Equal(y,rhs.y) && AEIsF32Equal(z,rhs.z) && AEIsF32Equal(w,rhs.w));
}
bool Point4::operator!=(const Point4& rhs) const {
  return (!AEIsF32Equal(x,rhs.x) || !AEIsF32Equal(y,rhs.y) || !AEIsF32Equal(z,rhs.z) || !AEIsF32Equal(w,rhs.w));
}

void Point4::Zero(void) {
  x = y = z = 0.0f;
  w = 1.0f;
}
void Point4::Print(void) const
{
  printf("%5.3f, %5.3f, %5.3f, %5.3f\n",x,y,z,w);
}