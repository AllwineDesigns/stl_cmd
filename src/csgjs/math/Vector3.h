#ifndef __CSGJS_VECTOR3__
#define __CSGJS_VECTOR3__

#include <iostream>
#include <cmath>
#include <algorithm>
#include "csgjs/math/Matrix4x4.h"

namespace csgjs {

struct Vector3 {
  csgjs_real x;
  csgjs_real y;
  csgjs_real z;

  Vector3();
  Vector3(csgjs_real xx, csgjs_real yy, csgjs_real zz);

  Vector3 operator-() const;
  Vector3 operator+(const Vector3 &b) const;
  Vector3 operator-(const Vector3 &b) const;
  Vector3 operator*(const csgjs_real m) const;
  Vector3 operator/(const csgjs_real m) const;
  csgjs_real dot(const Vector3 &b) const;
  Vector3 lerp(const Vector3 &b, csgjs_real t) const;
  csgjs_real lengthSquared() const;
  csgjs_real length() const;
  Vector3 unit() const;
  Vector3 cross(const Vector3 &a) const;
  csgjs_real distanceTo(const Vector3 &a) const;
  csgjs_real distanceToSquared(const Vector3 &a) const;
  bool operator==(const Vector3 &a) const;
  Vector3 transform(const Matrix4x4 &m, csgjs_real w=1) const;
  Vector3 abs() const;
  Vector3 nonParallelVector() const;
  Vector3 min(const Vector3 &v) const;
  Vector3 max(const Vector3 &v) const;
  bool isZero() const;
};

Vector3 operator*(const csgjs_real m, const Vector3 &v);
std::ostream& operator<<(std::ostream& os, const Vector3 &v);
}


#endif
