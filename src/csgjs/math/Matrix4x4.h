#ifndef __CSGJS_MATRIX4x4__
#define __CSGJS_MATRIX4x4__

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
  float m[16];

  Matrix4x4();
  Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

  bool isMirroring() const;

  Matrix4x4 operator+(const Matrix4x4 &mat) const;
  Matrix4x4 operator-(const Matrix4x4 &mat) const;
  Matrix4x4 operator*(const Matrix4x4 &mat) const;
  static Matrix4x4 translate(float x, float y, float z);
};

}

#endif
