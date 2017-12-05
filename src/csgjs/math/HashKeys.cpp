#include "csgjs/math/HashKeys.h"

namespace csgjs {
  LineKey::LineKey(const Line& l) {
    // A point and direction define a line. We want any equal Line to have the same hash value, so we need
    // a consistent way to represent a line, such that any Line we pass to this constructor will yield the 
    // same point and direction. We'll store the point that intersects the plane that intersects the origin, 
    // whose normal is the same as the direction of the line. We'll compare the direction of the line to 
    // nonParallelVector() to determine a consistent direction (store the direction that points in roughly the 
    // same direction as nonParallelVector())

    // We eliminate floating point errors by rounding to integers after scaling up by 1./EPS
    Vector3 d = l.direction.unit();

    Vector3 point = l.point-d*(d.dot(l.point));

    if(d.dot(d.abs()) >= 0) {
      line = Line(point, d);
    } else {
      line = Line(point, -d);
    }

    int x = (int)(std::round(line.direction.x/(10*EPS)));
    int y = (int)(std::round(line.direction.y/(10*EPS)));
    int z = (int)(std::round(line.direction.z/(10*EPS)));

    int x2 = (int)(std::round(line.point.x/(10*EPS)));
    int y2 = (int)(std::round(line.point.y/(10*EPS)));
    int z2 = (int)(std::round(line.point.z/(10*EPS)));

    hash = (((std::hash<int>()(x) ^ (std::hash<int>()(y) << 1)) >> 1) ^ (std::hash<int>()(z) << 1)) ^
           (((std::hash<int>()(x2) ^ (std::hash<int>()(y2) << 1)) >> 1) ^ (std::hash<int>()(z2) << 1));

  }

  bool LineKey::operator==(const LineKey &l) const {
    return l.line == line;
  }

  EdgeKey::EdgeKey(const Vector3 &a, const Vector3 &b) {
    first = a;
    second = b;

    int x = (int)(std::round(first.x/(10*EPS)));
    int y = (int)(std::round(first.y/(10*EPS)));
    int z = (int)(std::round(first.z/(10*EPS)));

    int x2 = (int)(std::round(second.x/(10*EPS)));
    int y2 = (int)(std::round(second.y/(10*EPS)));
    int z2 = (int)(std::round(second.z/(10*EPS)));

    hash = (((std::hash<int>()(x) ^ (std::hash<int>()(y) << 1)) >> 1) ^ (std::hash<int>()(z) << 1)) ^
           (((std::hash<int>()(x2) ^ (std::hash<int>()(y2) << 1)) >> 1) ^ (std::hash<int>()(z2) << 1));
  }

  EdgeKey EdgeKey::reversed() const {
    return EdgeKey(second, first);
  }

  bool EdgeKey::operator==(const EdgeKey &k) const {
    return (k.first-first).length() < EPS && (k.second-second).length() < EPS;
  }

  VertexKey::VertexKey(const Vector3 &a) : v(a) {
    int x = (int)(std::round(v.x/(10*EPS)));
    int y = (int)(std::round(v.y/(10*EPS)));
    int z = (int)(std::round(v.z/(10*EPS)));
    
    hash = (((std::hash<int>()(x) ^ (std::hash<int>()(y) << 1)) >> 1) ^ (std::hash<int>()(z) << 1));
  }

  bool VertexKey::operator==(const VertexKey &a) const {
    return (a.v-v).length() < EPS;
  }

  VertexKeyDist::VertexKeyDist(const VertexKey &k, csgjs_real b) : key(k), dist(b) {
  }

  bool VertexKeyDist::operator<(const VertexKeyDist &k) const {
    return dist-k.dist < -EPS;
  }

  bool VertexKeyDist::operator==(const VertexKeyDist &k) const {
    return key == k.key;
  }
}

std::size_t std::hash<csgjs::LineKey>::operator()(const csgjs::LineKey& l) const {
  return l.hash;
}

std::size_t std::hash<csgjs::EdgeKey>::operator()(const csgjs::EdgeKey& e) const {
  return e.hash;
}

std::size_t std::hash<csgjs::VertexKey>::operator()(const csgjs::VertexKey& v) const {
  return v.hash;
}

std::size_t std::hash<csgjs::VertexKeyDist>::operator()(const csgjs::VertexKeyDist& v) const {
  return v.key.hash;
}
