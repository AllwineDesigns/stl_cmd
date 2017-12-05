#ifndef __CSGJS_LINE3__
#define __CSGJS_LINE3__

#include "csgjs/math/Vector3.h"
#include <functional>

namespace csgjs {

  struct Line {
    Vector3 direction;
    Vector3 point;

    Line();
    Line(const Vector3 &p, const Vector3 &d);

    Vector3 closestPointOnLine(const Vector3 &p) const;
    csgjs_real distanceToPoint(const Vector3 &p) const;
    csgjs_real distanceToPointOnLine(const Vector3 &p) const;
    bool operator==(const Line &l) const;

    static Line fromPoints(const Vector3 &p1, const Vector3 &p2);
  };


  std::ostream& operator<<(std::ostream& os, const Line &line);

}

#endif
