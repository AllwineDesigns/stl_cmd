#include "csgjs/math/Matrix4x4.h"
#include "csgjs/math/Vector3.h"

namespace csgjs {

  Matrix4x4::Matrix4x4() {
    // identity matrix
    m[0] = 1;
    m[1] = 0;
    m[2] = 0;
    m[3] = 0;

    m[4] = 0;
    m[5] = 1;
    m[6] = 0;
    m[7] = 0;

    m[8] = 0;
    m[9] = 0;
    m[10] = 1;
    m[11] = 0;

    m[12] = 0;
    m[13] = 0;
    m[14] = 0;
    m[15] = 1;
  }
  // mrc - m<row><col>
  Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33) {
    // identity matrix
    m[0] = m00;
    m[1] = m01;
    m[2] = m02;
    m[3] = m03;

    m[4] = m10;
    m[5] = m11;
    m[6] = m12;
    m[7] = m13;

    m[8] = m20;
    m[9] = m21;
    m[10] = m22;
    m[11] = m23;

    m[12] = m30;
    m[13] = m31;
    m[14] = m32;
    m[15] = m33;
  }

  // this seems more consistent with how transforms are applied
  // but I transposed from what was in CSG.js, would
  // love for someone to tell me if I'm wrong or if it doesn't matter
  bool Matrix4x4::isMirroring() const {
    Vector3 u(m[0], m[1], m[2]);
    Vector3 v(m[4], m[5], m[6]);
    Vector3 w(m[8], m[9], m[10]);

    return u.cross(v).dot(w) < 0;
  }

  Matrix4x4 Matrix4x4::operator+(const Matrix4x4 &mat) const {
    return Matrix4x4(m[0]+mat.m[0], 
                  m[1]+mat.m[1],
                  m[2]+mat.m[2],
                  m[3]+mat.m[3],
                  m[4]+mat.m[4],
                  m[5]+mat.m[5],
                  m[6]+mat.m[6],
                  m[7]+mat.m[7],
                  m[8]+mat.m[8],
                  m[9]+mat.m[9],
                  m[10]+mat.m[10],
                  m[11]+mat.m[11],
                  m[12]+mat.m[12],
                  m[13]+mat.m[13],
                  m[14]+mat.m[14],
                  m[15]+mat.m[15]);
  }

  Matrix4x4 Matrix4x4::operator-(const Matrix4x4 &mat) const {
    return Matrix4x4(m[0]-mat.m[0], 
                  m[1]-mat.m[1],
                  m[2]-mat.m[2],
                  m[3]-mat.m[3],
                  m[4]-mat.m[4],
                  m[5]-mat.m[5],
                  m[6]-mat.m[6],
                  m[7]-mat.m[7],
                  m[8]-mat.m[8],
                  m[9]-mat.m[9],
                  m[10]-mat.m[10],
                  m[11]-mat.m[11],
                  m[12]-mat.m[12],
                  m[13]-mat.m[13],
                  m[14]-mat.m[14],
                  m[15]-mat.m[15]);
  }

  Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &mat) const {
    const float a00 = m[0];
    const float a01 = m[1];
    const float a02 = m[2];
    const float a03 = m[3];
    const float a10 = m[4];
    const float a11 = m[5];
    const float a12 = m[6];
    const float a13 = m[7];
    const float a20 = m[8];
    const float a21 = m[9];
    const float a22 = m[10];
    const float a23 = m[11];
    const float a30 = m[12];
    const float a31 = m[13];
    const float a32 = m[14];
    const float a33 = m[15];

    const float b00 = mat.m[0];
    const float b01 = mat.m[1];
    const float b02 = mat.m[2];
    const float b03 = mat.m[3];
    const float b10 = mat.m[4];
    const float b11 = mat.m[5];
    const float b12 = mat.m[6];
    const float b13 = mat.m[7];
    const float b20 = mat.m[8];
    const float b21 = mat.m[9];
    const float b22 = mat.m[10];
    const float b23 = mat.m[11];
    const float b30 = mat.m[12];
    const float b31 = mat.m[13];
    const float b32 = mat.m[14];
    const float b33 = mat.m[15];
    
    return Matrix4x4(a00*b00+a01*b10+a02*b20+a03*b30,
                     a00*b01+a01*b11+a02*b21+a03*b31,
                     a00*b02+a01*b12+a02*b22+a03*b32,
                     a00*b03+a01*b13+a02*b23+a03*b33,
                     a10*b00+a11*b10+a12*b20+a13*b30,
                     a10*b01+a11*b11+a12*b21+a13*b31,
                     a10*b02+a11*b12+a12*b22+a13*b32,
                     a10*b03+a11*b13+a12*b23+a13*b33,
                     a20*b00+a21*b10+a22*b20+a23*b30,
                     a20*b01+a21*b11+a22*b21+a23*b31,
                     a20*b02+a21*b12+a22*b22+a23*b32,
                     a20*b03+a21*b13+a22*b23+a23*b33,
                     a30*b00+a31*b10+a32*b20+a33*b30,
                     a30*b01+a31*b11+a32*b21+a33*b31,
                     a30*b02+a31*b12+a32*b22+a33*b32,
                     a30*b03+a31*b13+a32*b23+a33*b33);
  }

  Matrix4x4 Matrix4x4::translate(float x, float y, float z) {
    return Matrix4x4(1,0,0,0,
                     0,1,0,0,
                     0,0,1,0,
                     x,y,z,1);
  }

}
