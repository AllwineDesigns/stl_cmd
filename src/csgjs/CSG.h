#ifndef __CSGJS__CSG__
#define __CSGJS__CSG__

#include "csgjs/math/Polygon3.h"
#include "csgjs/math/Matrix4x4.h"
#include <vector>
#include <utility>

namespace csgjs {

class CSG {
  private:
    std::vector<Polygon> _polygons;
    bool _isCanonicalized;
    bool _isRetesselated;

    mutable bool _boundingBoxCacheValid;
    mutable std::pair<Vector3, Vector3> _boundingBoxCache;

    CSG unionForNonIntersecting(const CSG &csg) const;

  public:
    CSG();
    CSG(const std::vector<Polygon> &p, bool c=false, bool r=false);
    CSG(std::vector<Polygon> &&p, bool c=false, bool r=false);

    std::vector<Polygon> toPolygons() const;
    CSG csgUnion(const CSG &csg, bool retesselate=false, bool canonicalize=false) const;
    CSG csgIntersect(const CSG &csg, bool retesselate=false, bool canonicalize=false) const;
    CSG csgSubtract(const CSG &csg, bool retesselate=false, bool canonicalize=false) const;
    bool mayOverlap(const CSG &csg) const;
    std::pair<Vector3, Vector3> getBounds() const;

    CSG transform(const Matrix4x4 &m);

    friend std::ostream& operator<<(std::ostream& os, const CSG &csg);
};

}
#endif
