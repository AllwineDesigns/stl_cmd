#ifndef __CSGJS_MATRIX4x4__
#define __CSGJS_MATRIX4x4__

#include "csgjs/constants.h"

// row major implmentation of 4x4 matrix
// i.e. values stored as [ a b c d e f g h i j k l m n o p ]
// represent:
// | a b c d |
// | e f g h |
// | i j k l |
// | m n o p |
//
// This means that you left multiply to transform a point/vector:
//
// [ x y z 1 ] * | xx xy xz 0 | = [ x*xx+y*yx+z*zx+tx, x*xy+y*yy+z*zy+ty, x*xz+y*yz+z*zz+tz ]
//               | yx yy yz 0 |
//               | zx zy zz 0 |
//               | tx ty tz 1 |
//

namespace csgjs {

struct Vector3;

struct Matrix4x4 {
  csgjs_real m[16];

  Matrix4x4();
  Matrix4x4(csgjs_real m00, csgjs_real m01, csgjs_real m02, csgjs_real m03, csgjs_real m10, csgjs_real m11, csgjs_real m12, csgjs_real m13, csgjs_real m20, csgjs_real m21, csgjs_real m22, csgjs_real m23, csgjs_real m30, csgjs_real m31, csgjs_real m32, csgjs_real m33);

  bool isMirroring() const;

  Matrix4x4 operator+(const Matrix4x4 &mat) const;
  Matrix4x4 operator-(const Matrix4x4 &mat) const;
  Matrix4x4 operator*(const Matrix4x4 &mat) const;
  static Matrix4x4 translate(csgjs_real x, csgjs_real y, csgjs_real z);
  static Matrix4x4 rotate(const Vector3 &axis, csgjs_real angle);
};

}

#endif
