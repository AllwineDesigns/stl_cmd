#include "csgjs/math/HashKeys.h"

namespace csgjs {
  LineKey::LineKey(const Line& l) {
    // A point and direction define a line. We want any equal Line to have the same hash value, so we need
    // a consistent way to represent a line, such that any Line we pass to this constructor will yield the 
    // same point and direction. We'll store the point that intersects the plane that intersects the origin, 
    // whose normal is the same as the direction of the line. We'll compare the direction of the line to 
    // Vector(0,0,1) to determine a consistent direction (store the direction that points in roughly the 
    // same direction as Vector(0,0,1))

    // We eliminate floating point errors by rounding to integers after scaling up by 1./EPS
    Vector3 d = l.direction.unit();

    Vector3 point = l.point-d*(d.dot(l.point));

    float dotZ = d.dot(Vector3(0,0,1));
    if(dotZ > EPS) {
      line = Line(point, d);
    } else if(dotZ < -EPS) {
      line = Line(point, -d);
    } else {
      float dotY = d.dot(Vector3(0,1,0));
      if(dotY > EPS) {
        line = Line(point, d);
      } else if(dotY < -EPS) {
        line = Line(point, -d);
      } else {
        float dotX = d.dot(Vector3(1,0,0));
        if(dotX > EPS) {
          line = Line(point, d);
        } else {
          line = Line(point, -d);
        }
      }
    }

    long int x = (long int)(std::round(line.direction.x/(10*EPS)));
    long int y = (long int)(std::round(line.direction.y/(10*EPS)));
    long int z = (long int)(std::round(line.direction.z/(10*EPS)));

    long int x2 = (long int)(std::round(line.point.x/(10*EPS)));
    long int y2 = (long int)(std::round(line.point.y/(10*EPS)));
    long int z2 = (long int)(std::round(line.point.z/(10*EPS)));

//    hash = (((std::hash<long int>()(x) ^ (std::hash<long int>()(y) << 1)) >> 1) ^ (std::hash<long int>()(z) << 1)) ^
//           (((std::hash<long int>()(x2) ^ (std::hash<long int>()(y2) << 1)) >> 1) ^ (std::hash<long int>()(z2) << 1));
    hash = (std::hash<long int>()(x) ^ std::hash<long int>()(y) ^ std::hash<long int>()(z)) ^
           (std::hash<long int>()(x2) ^ std::hash<long int>()(y2) ^ std::hash<long int>()(z2));
  }

  bool LineKey::operator==(const LineKey &l) const {
    return l.line == line;
  }

  EdgeKey::EdgeKey(const Vector3 &a, const Vector3 &b) {
    first = a;
    second = b;

    long int x = (long int)(std::round(first.x/(10*EPS)));
    long int y = (long int)(std::round(first.y/(10*EPS)));
    long int z = (long int)(std::round(first.z/(10*EPS)));

    long int x2 = (long int)(std::round(second.x/(10*EPS)));
    long int y2 = (long int)(std::round(second.y/(10*EPS)));
    long int z2 = (long int)(std::round(second.z/(10*EPS)));

//    hash = (((std::hash<long int>()(x) ^ (std::hash<long int>()(y) << 1)) >> 1) ^ (std::hash<long int>()(z) << 1)) ^
//           (((std::hash<long int>()(x2) ^ (std::hash<long int>()(y2) << 1)) >> 1) ^ (std::hash<long int>()(z2) << 1));
    hash = (std::hash<long int>()(x) ^ std::hash<long int>()(y) ^ std::hash<long int>()(z)) ^
           (std::hash<long int>()(x2) ^ std::hash<long int>()(y2) ^ std::hash<long int>()(z2));
  }

  EdgeKey EdgeKey::reversed() const {
    return EdgeKey(second, first);
  }

  bool EdgeKey::operator==(const EdgeKey &k) const {
    return (k.first-first).length() < EPS && (k.second-second).length() < EPS;
  }

  VertexKey::VertexKey(const Vector3 &a) : v(a) {
    long int x = (long int)(std::round(v.x/(10*EPS)));
    long int y = (long int)(std::round(v.y/(10*EPS)));
    long int z = (long int)(std::round(v.z/(10*EPS)));
    
//    hash = (((std::hash<long int>()(x) ^ (std::hash<long int>()(y) << 1)) >> 1) ^ (std::hash<long int>()(z) << 1));
    hash = (std::hash<long int>()(x) ^ std::hash<long int>()(y) ^ std::hash<long int>()(z));
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
