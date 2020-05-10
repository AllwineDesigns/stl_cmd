#include "CSG.h"
#include "Trees.h"
#include <set>
#include <unordered_set>

namespace csgjs {
  CSG::CSG() : _boundingBoxCacheValid(false) {}
  CSG::CSG(const std::vector<Polygon> &p) : _polygons(p), _boundingBoxCacheValid(false) { }
  CSG::CSG(std::vector<Polygon> &&p) : _polygons(p), _boundingBoxCacheValid(false) { }

  std::vector<Polygon> CSG::toPolygons() const {
    return _polygons;
  }

  CSG CSG::csgUnion(const CSG &csg) const {
    if(!mayOverlap(csg)) {
      return unionForNonIntersecting(csg);
    }

    Tree A(_polygons);
    Tree B(csg._polygons);

    A.clipTo(B);
    B.clipTo(A);
    B.invert();
    B.clipTo(A);
    B.invert();

    std::vector<Polygon> aPolys(A.toPolygons());
    std::vector<Polygon> bPolys(B.toPolygons());

    aPolys.insert(aPolys.end(), bPolys.begin(), bPolys.end());

    return CSG(std::move(aPolys));
  }

  CSG CSG::csgIntersect(const CSG &csg) const {
    if(!mayOverlap(csg)) {
      return CSG();
    }

    Tree A(_polygons);
    Tree B(csg._polygons);

    A.invert();
    B.clipTo(A);
    B.invert();
    A.clipTo(B);
    B.clipTo(A);
    A.addPolygons(B.toPolygons());
    A.invert();

    std::vector<Polygon> aPolys(A.toPolygons());

    return CSG(std::move(aPolys));
  }

  CSG CSG::csgSubtract(const CSG &csg) const {
    if(!mayOverlap(csg)) {
      return *this;
    }

    Tree A(_polygons);
    Tree B(csg._polygons);

    A.invert();
    A.clipTo(B);
    B.clipTo(A, true);
    A.addPolygons(B.toPolygons());
    A.invert();

    std::vector<Polygon> aPolys(A.toPolygons());

    return CSG(std::move(aPolys));
  }

  CSG CSG::unionForNonIntersecting(const CSG &csg) const {
    std::vector<Polygon> all_polys;
    all_polys.reserve(_polygons.size()+csg._polygons.size());

    all_polys.insert(all_polys.end(), _polygons.begin(), _polygons.end());
    all_polys.insert(all_polys.end(), csg._polygons.begin(), csg._polygons.end());
    return CSG(std::move(all_polys));
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
          _boundingBoxCache.second = _boundingBoxCache.second.max(bounds.second);
        }

        ++itr;
      }

      _boundingBoxCacheValid = true;
    }
    return _boundingBoxCache;
  }

  CSG CSG::transform(const Matrix4x4 &mat) {
    std::vector<Polygon> newPolygons(_polygons);
    std::vector<Polygon>::iterator itr = newPolygons.begin();
    while(itr != newPolygons.end()) {
      *itr = itr->transform(mat);
      ++itr;
    }

    return CSG(std::move(newPolygons));
  }

  void CSG::findUnmatchedEdges(std::unordered_map<EdgeKey, PolygonEdgeData> &unmatchedEdges) {
    std::vector<Polygon>::iterator polyItr = _polygons.begin();
    while(polyItr != _polygons.end()) {
      std::vector<Vertex>::iterator vertexItr = polyItr->vertices.begin();
      while(vertexItr != polyItr->vertices.end()) {
        std::vector<Vertex>::iterator nextVertexItr = vertexItr+1;
        if(nextVertexItr == polyItr->vertices.end()) {
          nextVertexItr = polyItr->vertices.begin();
        }
        EdgeKey edgeKey(vertexItr->pos, nextVertexItr->pos);
        EdgeKey reversedKey = edgeKey.reversed();

        if(unmatchedEdges.count(reversedKey) == 0) {
          unmatchedEdges[edgeKey] = PolygonEdgeData(&(*polyItr), vertexItr->pos, nextVertexItr->pos);
        } else {
          unmatchedEdges.erase(reversedKey);
        }

        ++vertexItr;
      }

      ++polyItr;
    }
  }

  void CSG::canonicalize() {
    std::unordered_map<VertexKey, Vector3> vertexLookup;

    std::vector<Polygon>::iterator polyItr = _polygons.begin();
    while(polyItr != _polygons.end()) {
      std::vector<Vertex>::iterator vertexItr = polyItr->vertices.begin();
      while(vertexItr != polyItr->vertices.end()) {
        Vector3 v = vertexItr->pos;
        VertexKey k(v);

        if(vertexLookup.count(k) > 0) {
          v = vertexLookup[k];
        } else {
          vertexLookup[k] = v;
        }

        vertexItr->pos = v;
        ++vertexItr;
      }

      ++polyItr;
    }


  }

  void CSG::makeManifold() {
    // 1. Create an empty set of edges, E. Iterate over every edge of every polygon, adding each to E if its reverse edge hasn't already been added or removing the reverse edge them if it has.
    //    This will leave only edges that don't have a matching edge (a neighboring polygon, with the same edge).
    // 2. Add all colinear edges in unmatchedEdges to its own list
    // 3. Iterate over each set of edges, creating a lookup table of start and end vertices for each edge.
    //    Also, keep a sorted list of vertices based on their position along the line.
    // 4. Keep track of active edges while iterating over each vertex in order. An active edge is one where we've encountered it's first verex, but not it's second.
    // 5. For each vertex we encounter, split the currently active edges into 2 (edge A-------B will be come A----V----B, AB -> AV and VB), making sure to update the edge's corresponding polygon.

    // 1. E = unmatchedEdges
    std::unordered_map<EdgeKey, PolygonEdgeData> unmatchedEdges;
    findUnmatchedEdges(unmatchedEdges);

    // 2. Add all colinear edges in unmatchedEdges to its own list
    std::unordered_map<LineKey, std::vector<PolygonEdgeData*>> edgesPerLine;

    std::unordered_map<EdgeKey, PolygonEdgeData>::iterator edgeItr = unmatchedEdges.begin();
    while(edgeItr != unmatchedEdges.end()) {
      LineKey lineKey(Line::fromPoints(edgeItr->first.first, edgeItr->first.second));

      edgesPerLine[lineKey].push_back(&(edgeItr->second));

      ++edgeItr;
    }

    // 3.
    std::unordered_map<LineKey, std::vector<PolygonEdgeData*>>::iterator lineItr = edgesPerLine.begin();

    int lineNum = 0;
    //std::cout << edgesPerLine.size() << " lines" << std::endl;
    while(lineItr != edgesPerLine.end()) {
      //std::cout << lineNum << " " << lineItr->first.line << " " << lineItr->second.size() << std::endl;
      // for each line
      // create lookup tables for the start and end of each edge based on vertex position
      std::unordered_map<VertexKey, std::vector<PolygonEdgeData*> > startVertex2PolygonEdgeData;
      std::unordered_map<VertexKey, std::vector<PolygonEdgeData*> > endVertex2PolygonEdgeData;

      // keep a sorted set of vertices based on the position along the line
      std::set<VertexKeyDist> vertices;

      // keep track of the polygons that we need to insert them in reverse order
      std::unordered_set<Polygon*> insertForward;
      std::unordered_set<Polygon*> insertReversed;

      // for each edge on line
      std::vector<PolygonEdgeData*>::iterator edgeDataItr = lineItr->second.begin();
      while(edgeDataItr != lineItr->second.end()) {
        // insert into start and end lookup tables based on the order we'll encounter them
        // while iterating over the vertices from lowest to highest
        VertexKey firstKey = VertexKey((*edgeDataItr)->first);
        VertexKey secondKey = VertexKey((*edgeDataItr)->second);
        csgjs_real firstDist = lineItr->first.line.distanceToPointOnLine(firstKey.v);
        csgjs_real secondDist = lineItr->first.line.distanceToPointOnLine(secondKey.v);

        vertices.insert(VertexKeyDist(firstKey,firstDist));
        vertices.insert(VertexKeyDist(secondKey,secondDist));

        if(firstDist < secondDist) {
          startVertex2PolygonEdgeData[firstKey].push_back(*edgeDataItr);
          endVertex2PolygonEdgeData[secondKey].push_back(*edgeDataItr);
          insertForward.insert((*edgeDataItr)->polygon);
        } else {
          startVertex2PolygonEdgeData[secondKey].push_back(*edgeDataItr);
          endVertex2PolygonEdgeData[firstKey].push_back(*edgeDataItr);
          insertReversed.insert((*edgeDataItr)->polygon);
        }

        ++edgeDataItr;
      }

#ifdef CSGJS_DEBUG
      std::cout << "all vertices" << std::endl;
      std::set<VertexKeyDist>::iterator testItr = vertices.begin();
      while(testItr != vertices.end()) {
        std::cout << testItr->key.hash << " " << testItr->key.v << " " << testItr->dist << std::endl;
        ++testItr;
      }

      std::cout << "starting vertices " << std::endl;
      std::unordered_map<VertexKey, std::vector<PolygonEdgeData*> >::iterator startItr = startVertex2PolygonEdgeData.begin();
      while(startItr != startVertex2PolygonEdgeData.end()) {
        std::cout << startItr->first.hash << " " << startItr->first.v << " starts " << startItr->second.size() << " edges" << std::endl;
        ++startItr;
      }

      std::cout << "ending vertices " << std::endl;
      std::unordered_map<VertexKey, std::vector<PolygonEdgeData*> >::iterator endItr = endVertex2PolygonEdgeData.begin();
      while(endItr != endVertex2PolygonEdgeData.end()) {
        std::cout << endItr->first.hash << " " << endItr->first.v << " ends " << endItr->second.size() << " edges" << std::endl;
        ++endItr;
      }
#endif

      // keep track of vertices to add to each polygon, after we're done we'll iterate over this to actually insert them
      std::unordered_map<Polygon*, std::vector<std::pair<Vector3,Vector3> > > verticesToInsert;

      // keep track of which edges are active, which means we've encountered their start vertex, but not their end vertex
      std::unordered_set<PolygonEdgeData*> activeEdges;

      std::set<VertexKeyDist>::iterator vertexItr = vertices.begin();
      while(vertexItr != vertices.end()) {
        if(endVertex2PolygonEdgeData.count(vertexItr->key) > 0) {
          // we've encountered an end vertex, so remove all edges that ended from the activeEdges set
          std::vector<PolygonEdgeData*>::iterator endingItr = endVertex2PolygonEdgeData[vertexItr->key].begin();
          while(endingItr != endVertex2PolygonEdgeData[vertexItr->key].end()) {
            activeEdges.erase(*endingItr);
            ++endingItr;
          }
        }

        // loop over all activeEdges and indicate that we want to add the current vertex to its polygon
        std::unordered_set<PolygonEdgeData*>::iterator activeItr = activeEdges.begin();
        while(activeItr != activeEdges.end()) {
          PolygonEdgeData *edgeData = *activeItr;
          //std::cout << "indicating we want to insert " << vertexItr->key.v << " after " << edgeData->first << std::endl;
          verticesToInsert[edgeData->polygon].push_back(std::make_pair(vertexItr->key.v, edgeData->first));
          ++activeItr;
        }

        // add all edges that start on this vertex to the active edges set
        std::vector<PolygonEdgeData*>::iterator startingItr = startVertex2PolygonEdgeData[vertexItr->key].begin();
        while(startingItr != startVertex2PolygonEdgeData[vertexItr->key].end()) {
          activeEdges.insert(*startingItr);
          ++startingItr;
        }

        ++vertexItr;
      }
#ifdef CSGJS_DEBUG
      if(activeEdges.size() > 0) {
        std::cout << "still have " << activeEdges.size() << " edges in active list" << std::endl;
        std::unordered_set<PolygonEdgeData*>::iterator itr = activeEdges.begin();
        while(itr != activeEdges.end()) {
          std::cout << (*itr)->first << " " << (*itr)->second << std::endl;
          ++itr;
        }
        std::cout << "active edges set wasn't empty" << std::endl;
        throw new std::runtime_error("active edges set wasn't empty");
      }
#endif

      // insert vertices into polygons that were encountered in forward order
      std::unordered_set<Polygon*>::iterator forwardItr = insertForward.begin();
      while(forwardItr != insertForward.end()) {
        Polygon* polygon = *forwardItr;

        //std::cout << "inserting " << verticesToInsert[polygon].size() << " extra vertices" << std::endl;
        std::vector<std::pair<Vector3,Vector3> >::iterator insertItr = verticesToInsert[polygon].begin();
        //while(insertItr != verticesToInsert[polygon].end()) {
          //std::cout << *polygon << std::endl;
        //  //std::cout << "inserting" << insertItr->first << " after " << insertItr->second << std::endl;
        //  ++insertItr;
        //}
        //insertItr = verticesToInsert[polygon].begin();

        //std::cout << polygon->vertices << std::endl;

        std::vector<Vertex>::iterator vertexItr = polygon->vertices.begin();
        std::vector<Vertex> newVertices;
        while(vertexItr != polygon->vertices.end()) {
          newVertices.push_back(*vertexItr);
          while(insertItr != verticesToInsert[polygon].end() && vertexItr->pos == insertItr->second) {
            newVertices.push_back(Vertex(insertItr->first));
            ++insertItr;
          }
          ++vertexItr;
        }

        //std::cout << "replacing " << polygon->vertices.size() << " vertices with " << newVertices.size() << " vertices" << std::endl;
        polygon->vertices = std::move(newVertices);

        //std::cout << polygon->vertices << std::endl;

        ++forwardItr;
      }

      // insert vertices into polygons that were encountered in reverse order
      std::unordered_set<Polygon*>::iterator reverseItr =  insertReversed.begin();;
      while(reverseItr != insertReversed.end()) {
        Polygon* polygon = *reverseItr;

        //std::cout << "rev inserting " << verticesToInsert[polygon].size() << " extra vertices" << std::endl;
        std::vector<std::pair<Vector3,Vector3> >::reverse_iterator insertItr = verticesToInsert[polygon].rbegin();
        std::vector<Vertex>::iterator vertexItr = polygon->vertices.begin();
        std::vector<Vertex> newVertices;
        while(vertexItr != polygon->vertices.end()) {
          newVertices.push_back(*vertexItr);
          while(insertItr != verticesToInsert[polygon].rend() && vertexItr->pos == insertItr->second) {
            newVertices.push_back(Vertex(insertItr->first));
            ++insertItr;
          }
          ++vertexItr;
        }

        //std::cout << "rev replacing " << polygon->vertices.size() << " vertices with " << newVertices.size() << " vertices" << std::endl;
        polygon->vertices = std::move(newVertices);

        ++reverseItr;
      }

      ++lineItr;
      ++lineNum;
    }

#ifdef CSGJS_DEBUG
    std::unordered_map<EdgeKey, PolygonEdgeData> stillUnmatched;
    findUnmatchedEdges(stillUnmatched);

    if(stillUnmatched.size() > 0) {
      std::cout << "still " << stillUnmatched.size() << " unmatched edges" << std::endl;
      std::unordered_map<EdgeKey, PolygonEdgeData>::iterator itr = stillUnmatched.begin();
      while(itr != stillUnmatched.end()) {
        LineKey lineKey(Line::fromPoints(itr->first.first, itr->first.second));
        std::cout << itr->first.hash << " " << itr->second.first << " " << itr->second.second << " " << lineKey.hash << " " << lineKey.line << std::endl;

        ++itr;
      }

      std::cout << "still " << stillUnmatched.size() << " unmatched edges" << std::endl;
      throw new std::runtime_error("still unmatched edges");
    }
#endif
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
