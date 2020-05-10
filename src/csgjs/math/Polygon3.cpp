#include "csgjs/math/Polygon3.h"

#include <stdexcept>

namespace csgjs {
  Polygon::Polygon(const std::vector<Vertex> &v, const Plane &p) : _boundingSphereCacheValid(false), _boundingBoxCacheValid(false), vertices(v), plane(p) {
#ifdef CSGJS_DEBUG
    if(!checkIfConvex()) {
      std::cout << "not convex " << *this << std::endl;
//      throw std::runtime_error("Not convex!");
    }
#endif
  }

  Polygon::Polygon(std::vector<Vertex> &&v, const Plane &p) : _boundingSphereCacheValid(false), _boundingBoxCacheValid(false), vertices(v), plane(p) {
#ifdef CSGJS_DEBUG
    if(!checkIfConvex()) {
      std::cout << "not convex " << *this << std::endl;
//      throw std::runtime_error("Not convex!");
    }
#endif
  }

  Polygon::Polygon(const std::vector<Vertex> &v) : vertices(v), _boundingSphereCacheValid(false), _boundingBoxCacheValid(false) {
    plane = Plane::fromVector3s(vertices[0].pos, vertices[1].pos, vertices[2].pos);
#ifdef CSGJS_DEBUG
    if(!checkIfConvex()) {
      std::cout << "not convex " << *this << std::endl;
//      throw std::runtime_error("Not convex!");
    }
#endif
  }

  Polygon::Polygon() : _boundingSphereCacheValid(false), _boundingBoxCacheValid(false) {}

  Polygon Polygon::flipped() const {
    std::vector<Vertex> newVertices;
    Plane newPlane = plane.flipped();

    std::vector<Vertex>::const_reverse_iterator itr = vertices.rbegin();
    while(itr != vertices.rend()) {
      newVertices.push_back(itr->flipped());
      ++itr;
    }

    return Polygon(std::move(newVertices), newPlane);
  }

  bool Polygon::checkIfDegenerateTriangle() const {
    int numVertices = vertices.size();
    if(numVertices == 3) {
      Vector3 v1 = (vertices[2].pos-vertices[0].pos);
      Vector3 v2 = (vertices[1].pos-vertices[0].pos);
      Vector3 v3 = (vertices[2].pos-vertices[1].pos);

      double a = v1.length();
      double b = v2.length();
      double c = v3.length();

      if(a > c) {
        double tmp = c;
        c = a;
        a = tmp;
      }

      if(a > b) {
        double tmp = b;
        b = a;
        a = tmp;
      }

      if(b > c) {
        double tmp = c;
        c = b;
        b = tmp;
      }

      double d = a+b-c;

      return (d < EPS);
    }
    return false;
  }

  bool Polygon::checkIfConvex() const {
    int numVertices = vertices.size();
    if(numVertices > 2) {
      Vector3 prevprevpos = vertices[numVertices-2].pos;
      Vector3 prevpos = vertices[numVertices-1].pos;
      for(int i = 0; i < numVertices; i++) {
        Vector3 pos = vertices[i].pos;
        if(!Polygon::isConvexPoint(prevprevpos, prevpos, pos, plane.normal)) {
          return false;
        }
        prevprevpos = prevpos;
        prevpos = pos;
      }
    }
    return true;
  }

  Polygon Polygon::transform(const Matrix4x4 &m) const {
    std::vector<Vertex> verts(vertices);
    std::vector<Vertex>::iterator itr = verts.begin();
    while(itr != verts.end()) {
      *itr = itr->transform(m);
      ++itr;
    }

    if(m.isMirroring()) {
      std::reverse(verts.begin(), verts.end());
    }

    return Polygon(std::move(verts), plane.transform(m));
  }

  bool Polygon::isConvexPoint(const Vector3 &prevpoint, const Vector3 &point, const Vector3 &nextpoint, const Vector3 normal) {
    Vector3 crossproduct = (point-prevpoint).cross(nextpoint-point);
    csgjs_real crossdotnormal = crossproduct.dot(normal);
    return crossdotnormal >= 0;
  }

  // seems like this caching scheme could be an unnecessary optimization, but
  // this is how it was done in CSG.js, maybe try calculating the bounding box and sphere
  // in the constructor to avoid any branching (it probably makes sense in JavaScript,
  // but I question the benefit in C++). It would come down to how often polygons are
  // constructed without ever needing a bounding box or sphere, which I'm not sure about.
  std::pair<Vector3, Vector3> Polygon::boundingBox() const {
    if(!_boundingBoxCacheValid) {
      std::vector<Vertex>::const_iterator itr = vertices.begin();

      if(itr != vertices.end()) {
        _boundingBoxCache.first = itr->pos;
        _boundingBoxCache.second = itr->pos;

        while(itr != vertices.end()) {
          _boundingBoxCache.first = _boundingBoxCache.first.min(itr->pos);
          _boundingBoxCache.second = _boundingBoxCache.second.max(itr->pos);
          ++itr;
        }
        _boundingBoxCacheValid = true;

      } else {
        _boundingBoxCache.first = Vector3(0,0,0);
        _boundingBoxCache.second = Vector3(0,0,0);
        _boundingBoxCacheValid = true;
      }
    }

    return _boundingBoxCache;
  }

  std::pair<Vector3, csgjs_real> Polygon::boundingSphere() const {
    if(!_boundingSphereCacheValid) {
      std::pair<Vector3, Vector3> box = boundingBox();
      _boundingSphereCache.first = .5*(box.first+box.second);
      _boundingSphereCache.second = (box.second-_boundingSphereCache.first).length();
      _boundingSphereCacheValid = true;
    }

    return _boundingSphereCache;
  }

  PolygonEdgeData::PolygonEdgeData() : polygon(NULL) {
  }

  PolygonEdgeData::PolygonEdgeData(Polygon *p, const Vector3 &a, const Vector3 &b) : polygon(p), first(a), second(b) {
  }

  std::ostream& operator<<(std::ostream& os, const Polygon &poly) {
    os << "Polygon - vertices: { ";

    std::vector<Vertex>::const_iterator itr = poly.vertices.begin();
    while(itr != poly.vertices.end()) {
      os << *itr;
      ++itr;

      if(itr != poly.vertices.end()) {
        os << ", ";
      }
    }
    os << " }, plane: { " << poly.plane << " }";

    return os;
  }

  std::ostream& operator<<(std::ostream& os, const std::pair<Vector3, Vector3> &bounds) {
    os << bounds.first << " - " << bounds.second;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const std::pair<Vector3, csgjs_real> &bounds) {
    os << bounds.first << " - " << bounds.second;
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const std::vector<Vertex> &vertices) {
    std::vector<Vertex>::const_iterator itr = vertices.begin();
    while(itr != vertices.end()) {
      os << itr->pos << " ";
      ++itr;
    }
    return os;
  }
}

