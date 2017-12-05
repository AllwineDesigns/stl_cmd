#ifndef __CSGJS__CSG__
#define __CSGJS__CSG__

#include "csgjs/math/Polygon3.h"
#include "csgjs/math/Matrix4x4.h"
#include <vector>
#include <utility>
#include "csgjs/math/HashKeys.h"
#include <unordered_map>

namespace csgjs {

class CSG {
  private:
    std::vector<Polygon> _polygons;
    bool _isManifold;

    mutable bool _boundingBoxCacheValid;
    mutable std::pair<Vector3, Vector3> _boundingBoxCache;

    CSG unionForNonIntersecting(const CSG &csg) const;

    void findUnmatchedEdges(std::unordered_map<EdgeKey, PolygonEdgeData> &u);

  public:
    CSG();
    CSG(const std::vector<Polygon> &p);
    CSG(std::vector<Polygon> &&p);

    std::vector<Polygon> toPolygons() const;
    CSG csgUnion(const CSG &csg) const;
    CSG csgIntersect(const CSG &csg) const;
    CSG csgSubtract(const CSG &csg) const;
    bool mayOverlap(const CSG &csg) const;
    std::pair<Vector3, Vector3> getBounds() const;

    CSG transform(const Matrix4x4 &m);

    void canonicalize();
    void makeManifold();

    friend std::ostream& operator<<(std::ostream& os, const CSG &csg);
};

}
#endif
