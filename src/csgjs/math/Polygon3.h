#ifndef __CSGJS_POLYGON3__
#define __CSGJS_POLYGON3__

#include "csgjs/math/Vector3.h"
#include "csgjs/math/Vertex3.h"
#include "csgjs/math/Plane.h"
#include <vector>
#include <utility>

namespace csgjs {

class Polygon {
  public:
    std::vector<Vertex> vertices;
    Plane plane;

    Polygon(std::vector<Vertex> &&v, const Plane &p);
    Polygon(const std::vector<Vertex> &v, const Plane &p);
    Polygon(const std::vector<Vertex> &v);
    Polygon();

    bool checkIfDegenerateTriangle() const; // checks if polygon has 3 vertices and if they're a degenerate triangle
    bool checkIfConvex() const;
    std::pair<Vector3, csgjs_real> boundingSphere() const;
    std::pair<Vector3, Vector3> boundingBox() const;

    Polygon flipped() const;
    Polygon transform(const Matrix4x4 &m) const;

    static bool isConvexPoint(const Vector3 &prevpoint, const Vector3 &point, const Vector3 &nextpoint, const Vector3 normal);

  private:
    mutable bool _boundingSphereCacheValid;
    mutable std::pair<Vector3, csgjs_real> _boundingSphereCache;

    mutable bool _boundingBoxCacheValid;
    mutable std::pair<Vector3, Vector3> _boundingBoxCache;
};

struct PolygonEdgeData {
  Polygon *polygon;

  Vector3 first;
  Vector3 second;

  PolygonEdgeData();
  PolygonEdgeData(Polygon *p, const Vector3 &a, const Vector3 &b);
};


std::ostream& operator<<(std::ostream& os, const Polygon &poly);
std::ostream& operator<<(std::ostream& os, const std::pair<Vector3, Vector3> &bounds);
std::ostream& operator<<(std::ostream& os, const std::pair<Vector3, csgjs_real> &bounds);

std::ostream& operator<<(std::ostream& os, const std::vector<Vertex> &vertices);

}

#endif
