#ifndef __CSGJS_VERTEX3__
#define __CSGJS_VERTEX3__

#include "csgjs/math/Vector3.h"
#include <iostream>

namespace csgjs {
  struct Vertex {
    Vector3 pos;

    Vertex() : pos() {
    }

    Vertex(const Vector3 &p) : pos(p) {}

    Vertex flipped() const {
      return *this;
    }

    Vertex interpolate(Vertex other, float t) const {
      return Vertex(pos.lerp(other.pos, t));
    }

    Vertex transform(const Matrix4x4 &m) const {
      return Vertex(pos.transform(m));
    }


  };

  inline std::ostream& operator<<(std::ostream& os, const Vertex &v) {
    os << "pos: " << v.pos;
    return os;
  }
}

#endif
