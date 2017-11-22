#ifndef __CSGJS_VECTOR3__
#define __CSGJS_VECTOR3__

#include <iostream>
#include <cmath>
#include <algorithm>
#include "csgjs/math/Matrix4x4.h"

namespace csgjs {

struct Vector3 {
  float x; 
  float y; 
  float z;

  Vector3();
  Vector3(float xx, float yy, float zz);

  Vector3 operator-() const;
  Vector3 operator+(const Vector3 &b) const;
  Vector3 operator-(const Vector3 &b) const;
  Vector3 operator*(const float m) const;
  Vector3 operator/(const float m) const;
  float dot(const Vector3 &b) const;
  Vector3 lerp(const Vector3 &b, float t) const;
  float lengthSquared() const;
  float length() const;
  Vector3 unit() const;
  Vector3 cross(const Vector3 &a) const;
  float distanceTo(const Vector3 &a) const;
  float distanceToSquared(const Vector3 &a) const;
  bool operator==(const Vector3 &a) const;
  Vector3 transform(const Matrix4x4 &m, float w=1) const;
  Vector3 abs() const;
  Vector3 randomNonParallelVector() const;
  Vector3 min(const Vector3 &v) const;
  Vector3 max(const Vector3 &v) const;
};

Vector3 operator*(const float m, const Vector3 &v);
std::ostream& operator<<(std::ostream& os, const Vector3 &v);
}


#endif
