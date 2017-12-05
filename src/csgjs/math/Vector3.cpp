#include "csgjs/math/Vector3.h"

namespace csgjs {

  Vector3::Vector3() : x(0), y(0), z(0) {}
  Vector3::Vector3(csgjs_real xx, csgjs_real yy, csgjs_real zz) : x(xx), y(yy), z(zz) {}

  Vector3 Vector3::operator-() const {
    return Vector3(-x, -y, -z);
  }

  Vector3 Vector3::operator+(const Vector3 &b) const {
    return Vector3(x+b.x, y+b.y, z+b.z);
  }

  Vector3 Vector3::operator-(const Vector3 &b) const {
    return Vector3(x-b.x, y-b.y, z-b.z);
  }

  Vector3 Vector3::operator*(const csgjs_real m) const {
    return Vector3(m*x, m*y, m*z);
  }

  Vector3 Vector3::operator/(const csgjs_real m) const {
    return Vector3(x/m, y/m, z/m);
  }

  csgjs_real Vector3::dot(const Vector3 &b) const {
    return x*b.x+y*b.y+z*b.z;
  }

  Vector3 Vector3::lerp(const Vector3 &b, csgjs_real t) const {
    return (*this)+(b-*this)*t;
  }

  csgjs_real Vector3::lengthSquared() const {
    return this->dot(*this);
  }

  csgjs_real Vector3::length() const {
    return sqrt(this->lengthSquared());
  }

  Vector3 Vector3::unit() const {
    return (*this)/this->length();
  }

  Vector3 Vector3::cross(const Vector3 &a) const {
    return Vector3(y*a.z-z*a.y, z*a.x-x*a.z, x*a.y-y*a.x);
  }

  csgjs_real Vector3::distanceTo(const Vector3 &a) const {
    return ((*this)-a).length();
  }

  csgjs_real Vector3::distanceToSquared(const Vector3 &a) const {
    return ((*this)-a).lengthSquared();
  }

  bool Vector3::operator==(const Vector3 &a) const {
    return x == a.x && y == a.y && z == a.z;
  }

  Vector3 Vector3::transform(const Matrix4x4 &m, csgjs_real w) const {
    const csgjs_real xx = m.m[0];
    const csgjs_real xy = m.m[1];
    const csgjs_real xz = m.m[2];
    const csgjs_real xw = m.m[3];

    const csgjs_real yx = m.m[4];
    const csgjs_real yy = m.m[5];
    const csgjs_real yz = m.m[6];
    const csgjs_real yw = m.m[7];

    const csgjs_real zx = m.m[8];
    const csgjs_real zy = m.m[9];
    const csgjs_real zz = m.m[10];
    const csgjs_real zw = m.m[11];

    const csgjs_real tx = m.m[12];
    const csgjs_real ty = m.m[13];
    const csgjs_real tz = m.m[14];
    const csgjs_real tw = m.m[15];

    csgjs_real newX = x*xx+y*yx+z*zx+w*tx;
    csgjs_real newY = x*xy+y*yy+z*zy+w*ty;
    csgjs_real newZ = x*xz+y*yz+z*zz+w*tz;
    csgjs_real newW = x*xw+y*yw+z*zw+w*tw;

    if(newW != 1) {
      csgjs_real invW = 1.0/newW;
      newX *= invW;
      newY *= invW;
      newZ *= invW;
    }

    return Vector3( newX, newY, newZ );
  }

  Vector3 Vector3::abs() const {
    return Vector3(std::abs(x), std::abs(y), std::abs(z));
  }

  Vector3 Vector3::nonParallelVector() const {
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

  Vector3 operator*(const csgjs_real m, const Vector3 &v) {
    return Vector3(m*v.x, m*v.y, m*v.z);
  }

  std::ostream& operator<<(std::ostream& os, const Vector3 &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
  }

}
