#ifndef __CSGJS_PLANE__
#define __CSGJS_PLANE__

#include "csgjs/math/Vector3.h"
#include "csgjs/constants.h"

namespace csgjs {

class Plane;
inline std::ostream& operator<<(std::ostream& os, const Plane &plane);

struct Plane {
  Vector3 normal;
  csgjs_real w;

  Plane() : normal(), w(0) {}
  Plane(const Vector3 &n, const csgjs_real ww) : normal(n), w(ww) {}

  Plane flipped() const {
    return Plane(-normal, -w);
  }

  Plane transform(const Matrix4x4 &m) const {
    bool ismirror = m.isMirroring();

    Vector3 r = normal.randomNonParallelVector();
    Vector3 u = normal.cross(r);
    Vector3 v = normal.cross(u);

    Vector3 p1 = normal*w;
    Vector3 p2 = p1+u;
    Vector3 p3 = p1+v;

    Plane newPlane = Plane::fromVector3s(p1,p2,p3);
    if(ismirror) {
      newPlane = newPlane.flipped();
    }

    return newPlane;
  }

  Vector3 splitLineBetweenPoints(const Vector3 &p1, const Vector3 &p2) const {
    Vector3 dir = p2-p1;
    csgjs_real dot = normal.dot(dir);
    csgjs_real lambda = 0;
    if(dot > EPS || dot < NEG_EPS) {
      lambda = (w-normal.dot(p1))/dot;
    }
    if(lambda > 1) {
      lambda = 1;
    }

    if(lambda < 0) {
      lambda = 0;
    }

    Vector3 inter = p1+dir*lambda;

    return p1+dir*lambda;
  }

  bool operator==(const Plane &p) const {
    return normal == p.normal && w == p.w;
  }

  static Plane fromVector3s(const Vector3 &a, const Vector3 &b, const Vector3 &c) {
    Vector3 n = ((b-a).cross(c-a)).unit();
    return Plane(n, n.dot(a));
  }

  static Plane anyPlaneFromVector3s(const Vector3 &a, const Vector3 &b, const Vector3 &c) {
    Vector3 v1 = b-a;
    Vector3 v2 = c-a;
    if(v1.length() < EPS) {
      v1 = v2.randomNonParallelVector();
    }
    if(v2.length() < EPS) {
      v2 = v1.randomNonParallelVector();
    }

    Vector3 normal = v1.cross(v2);
    if(normal.length() < EPS) {
      // this means that v1 == -v2
      v2 = v1.randomNonParallelVector();
      normal = v1.cross(v2);
    }
    normal = normal.unit();
    return Plane(normal, normal.dot(a));
  }

  static Plane fromNormalAndPoint(const Vector3 &n, const Vector3 &p) {
    Vector3 normal = n.unit();
    csgjs_real w = p.dot(normal);
    return Plane(normal, w);
  }

};

inline std::ostream& operator<<(std::ostream& os, const Plane &plane) {
  os << plane.normal << ", " << plane.w;
  return os;
}


}

#endif
