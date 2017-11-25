#include "csgjs/math/Vector3.h"

namespace csgjs {

  Vector3::Vector3() : x(0), y(0), z(0) {}
  Vector3::Vector3(float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}

  Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
  }

  Vector3 Vector3::operator+(const Vector3 &b) const {
    return Vector3(x+b.x, y+b.y, z+b.z);
  }

  Vector3 Vector3::operator-(const Vector3 &b) const {
    return Vector3(x-b.x, y-b.y, z-b.z);
  }

  Vector3 Vector3::operator*(const float m) const {
    return Vector3(m*x, m*y, m*z);
  }

  Vector3 Vector3::operator/(const float m) const {
    return Vector3(x/m, y/m, z/m);
  }

  float Vector3::dot(const Vector3 &b) const {
    return x*b.x+y*b.y+z*b.z;
  }

  Vector3 Vector3::lerp(const Vector3 &b, float t) const {
    return (*this)+(b-*this)*t;
  }

  float Vector3::lengthSquared() const {
    return this->dot(*this);
  }

  float Vector3::length() const {
    return sqrt(this->lengthSquared());
  }

  Vector3 Vector3::unit() const {
    return (*this)/this->length();
  }

  Vector3 Vector3::cross(const Vector3 &a) const {
    return Vector3(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
  }

  float Vector3::distanceTo(const Vector3 &a) const {
    return ((*this)-a).length();
  }

  float Vector3::distanceToSquared(const Vector3 &a) const {
    return ((*this)-a).lengthSquared();
  }

  bool Vector3::operator==(const Vector3 &a) const {
    return x == a.x && y == a.y && z == a.z;
  }

  Vector3 Vector3::transform(const Matrix4x4 &m, float w) const {
    const float xx = m.m[0];
    const float xy = m.m[1];
    const float xz = m.m[2];
    const float xw = m.m[3];

    const float yx = m.m[4];
    const float yy = m.m[5];
    const float yz = m.m[6];
    const float yw = m.m[7];

    const float zx = m.m[8];
    const float zy = m.m[9];
    const float zz = m.m[10];
    const float zw = m.m[11];

    const float tx = m.m[12];
    const float ty = m.m[13];
    const float tz = m.m[14];
    const float tw = m.m[15];

    float newX = x*xx+y*yx+z*zx+w*tx;
    float newY = x*xy+y*yy+z*zy+w*ty;
    float newZ = x*xz+y*yz+z*zz+w*tz;
    float newW = x*xw+y*yw+z*zw+w*tw;

    if(newW != 1) {
      float invW = 1.0/newW;
      newX *= invW;
      newY *= invW;
      newZ *= invW;
    }

    return Vector3( newX, newY, newZ );
  }

  Vector3 Vector3::abs() const {
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
  }

  Vector3 Vector3::randomNonParallelVector() const {
    Vector3 abs = this->abs();
    if(abs.x <= abs.y && abs.x <= abs.z) {
      return Vector3(1,0,0);
    } else if(abs.y <= abs.x && abs.y <= abs.z) {
      return Vector3(0,1,0);
    } else {
      return Vector3(0,0,1);
    }
  }

  Vector3 Vector3::min(const Vector3 &v) const {
    return Vector3(std::min(x, v.x), std::min(y, v.y), std::min(z, v.z));
  }

  Vector3 Vector3::max(const Vector3 &v) const {
    return Vector3(std::max(x, v.x), std::max(y, v.y), std::max(z, v.z));
  }

  Vector3 operator*(const float m, const Vector3 &v) {
    return Vector3(m*v.x, m*v.y, m*v.z);
  }

  std::ostream& operator<<(std::ostream& os, const Vector3 &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
  }

}
