#include "CSG.h"

namespace csgjs {
  CSG::CSG() : _boundingBoxCacheValid(false), _isCanonicalized(true), _isRetesselated(true) {}
  CSG::CSG(const std::vector<Polygon> &p, bool c, bool r) : _polygons(p), _boundingBoxCacheValid(false), _isCanonicalized(c), _isRetesselated(r) { std::cout << "copied" << std::endl;}
  CSG::CSG(std::vector<Polygon> &&p, bool c, bool r) : _polygons(p), _boundingBoxCacheValid(false), _isCanonicalized(c), _isRetesselated(r) { std::cout << "moved" << std::endl;}

  std::vector<Polygon> CSG::toPolygons() const {
    return _polygons;
  }

  CSG CSG::csgUnion(const CSG &csg, bool retesselate, bool canonicalize) const {
    if(!mayOverlap(csg)) {
      return unionForNonIntersecting(csg);
    } else {
    }

    return CSG();
  }

  CSG CSG::unionForNonIntersecting(const CSG &csg) const {
    std::vector<Polygon> all_polys;
    all_polys.reserve(_polygons.size()+csg._polygons.size());

    all_polys.insert(all_polys.end(), _polygons.begin(), _polygons.end());
    all_polys.insert(all_polys.end(), csg._polygons.begin(), csg._polygons.end());
    return CSG(std::move(all_polys), _isCanonicalized && csg._isCanonicalized, _isRetesselated && csg._isRetesselated);
  }

  bool CSG::mayOverlap(const CSG &csg) const {
    if(_polygons.size() == 0 || csg._polygons.size() == 0) {
      return false;
    } else {
      std::pair<Vector3, Vector3> bounds = getBounds();
      std::pair<Vector3, Vector3> otherBounds = csg.getBounds();

      if(bounds.second.x < otherBounds.first.x) return false;
      if(bounds.first.x > otherBounds.second.x) return false;

      if(bounds.second.y < otherBounds.first.y) return false;
      if(bounds.first.y > otherBounds.second.y) return false;

      if(bounds.second.z < otherBounds.first.z) return false;
      if(bounds.first.z > otherBounds.second.z) return false;

      return true;
    }
  }

  std::pair<Vector3, Vector3> CSG::getBounds() const {
    if(!_boundingBoxCacheValid) {
      std::vector<Polygon>::const_iterator itr = _polygons.begin();
      while(itr != _polygons.end()) {
        std::pair<Vector3, Vector3> bounds = itr->boundingBox();

        if(itr == _polygons.begin()) {
          _boundingBoxCache.first = bounds.first;
          _boundingBoxCache.second = bounds.second;
        } else {
          _boundingBoxCache.first = _boundingBoxCache.first.min(bounds.first);
          _boundingBoxCache.second = _boundingBoxCache.second.min(bounds.second);
        }

        ++itr;
      }

      _boundingBoxCacheValid = true;
    }
    return _boundingBoxCache;
  }

  // since we're not using heap allocation for vertices/planes
  // and instead using passing by value for those, we're not going to try to look
  // up shared vertices/planes like CSG.js does and instead
  // just perform the transformation on everything
  CSG CSG::transform(const Matrix4x4 &mat) {
    std::vector<Polygon> newPolygons(_polygons);
    std::vector<Polygon>::iterator itr = newPolygons.begin();
    while(itr != newPolygons.end()) {
      *itr = itr->transform(mat);
      ++itr;
    }

    return CSG(std::move(newPolygons), false, _isRetesselated);
  }

  std::ostream& operator<<(std::ostream& os, const CSG &csg) {
    std::vector<Polygon>::const_iterator itr = csg._polygons.begin();
    os << "CSG {" << std::endl;
    while(itr != csg._polygons.end()) {
      os << *itr;

      ++itr;

      if(itr != csg._polygons.end()) {
        os << ",";
      }
      os << std::endl;
    }

    os << "}";
    return os;
  }
}
