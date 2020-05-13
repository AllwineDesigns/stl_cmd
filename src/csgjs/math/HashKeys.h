#ifndef __CSGJS_HASHKEYS__
#define __CSGJS_HASHKEYS__

#include "csgjs/math/Line3.h"
#include "csgjs/math/Plane.h"

namespace csgjs {
  struct PlaneKey {
    std::size_t hash;
    Plane plane;

    PlaneKey(const Plane &p);
    bool operator==(const PlaneKey &k) const;
  };

  struct LineKey {
    std::size_t hash;
    Line line;

    LineKey(const Line &l);
    bool operator==(const LineKey &l) const;
  };

  struct EdgeKey {
    std::size_t hash;

    Vector3 first;
    Vector3 second;

    EdgeKey(const Vector3 &a, const Vector3 &b);
    bool operator==(const EdgeKey &k) const;
    EdgeKey reversed() const;
  };

  struct VertexKey {
    std::size_t hash;

    Vector3 v;

    VertexKey(const Vector3 &a);
    bool operator==(const VertexKey &k) const;
  };

  struct VertexKeyDist {
    VertexKey key;
    csgjs_real dist;

    VertexKeyDist(const VertexKey &a, csgjs_real b);

    bool operator<(const VertexKeyDist &k) const;
    bool operator==(const VertexKeyDist &k) const;
  };
}

namespace std {
  template <>
  struct hash<csgjs::PlaneKey> {
    std::size_t operator()(const csgjs::PlaneKey& l) const;
  };

  template <>
  struct hash<csgjs::LineKey> {
    std::size_t operator()(const csgjs::LineKey& l) const;
  };

  template <>
  struct hash<csgjs::EdgeKey> {
    std::size_t operator()(const csgjs::EdgeKey& e) const;
  };

  template <>
  struct hash<csgjs::VertexKey> {
    std::size_t operator()(const csgjs::VertexKey& e) const;
  };

  template <>
  struct hash<csgjs::VertexKeyDist> {
    std::size_t operator()(const csgjs::VertexKeyDist& e) const;
  };
}

#endif
