#include "csgjs/math/Line3.h"
#include <iostream>

namespace csgjs {
  Line::Line() : point(Vector3(0,0,0)), direction(Vector3(0,0,0)) { // point and direction
  }

  Line::Line(const Vector3 &p, const Vector3 &d) : point(p), direction(d) { // point and direction
  }

  csgjs_real Line::distanceToPointOnLine(const Vector3 &p) const {
    return (p-point).dot(direction);
  }

  Vector3 Line::closestPointOnLine(const Vector3 &p) const {
    return point+direction*(p-point).dot(direction);
  }

  csgjs_real Line::distanceToPoint(const Vector3 &p) const {
    return (p-closestPointOnLine(p)).length();
  }

  bool Line::operator==(const Line &l) const {
    return (l.direction-direction).length() <= EPS && (l.point-point).length() <= EPS;
  }

  Line Line::fromPoints(const Vector3 &p1, const Vector3 &p2) {
    return Line(p1, p2-p1);
  }

  std::ostream& operator<<(std::ostream& os, const Line &line) {
    os << "Line - point: " << line.point << ", direction: " << line.direction;
    return os;
  }
}
