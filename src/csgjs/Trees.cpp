#include "csgjs/Trees.h"

namespace csgjs {
  Node::Node() : parent(NULL), front(NULL), back(NULL)  {
  }

  Node::Node(Node *p) : parent(p), front(NULL), back(NULL) {
  }

  void Node::addPolygonTreeNodes(const std::vector<PolygonTreeNode*> &polyTreeNodes) {
    std::vector<PolygonTreeNode*> frontNodes;
    std::vector<PolygonTreeNode*> backNodes;

//    int pick = fastRandom(polyTreeNodes.size());
    int pick = 0;
    plane = polyTreeNodes[pick]->getPolygon().plane;

    std::vector<PolygonTreeNode*>::const_iterator itr = polyTreeNodes.begin();
    while(itr != polyTreeNodes.end()) {
      // why is backNodes set to both coplanarBacknodes and backnodes?
      (*itr)->splitByPlane(plane, polygonTreeNodes, backNodes, frontNodes, backNodes);
      ++itr;
    }

    // CSG.js did this iteratively rather than recursively. Probably safer to do iteratively, but starting with a recursive
    // solution for ease of implementation. If it leads to stack overflows, will refactor. May see a performance improvement 
    // when implemented iteratively, so it might be worth trying.
    if(frontNodes.size() > 0) {
      front = new Node(this);
      front->addPolygonTreeNodes(frontNodes);
    }

    if(backNodes.size() > 0) {
      back = new Node(this);
      back->addPolygonTreeNodes(backNodes);
    }
  }

  bool Node::isRootNode() const {
    return parent == NULL;
  }

  Tree::Tree(const std::vector<Polygon> &polygons) {
    std::vector<Polygon>::const_iterator itr = polygons.begin();

    std::vector<PolygonTreeNode*> polyTreeNodes;
    polyTreeNodes.reserve(polygons.size());

    while(itr != polygons.end()) {
      polyTreeNodes.push_back(polygonTree.addChild(*itr));
      ++itr;
    }

    rootnode.addPolygonTreeNodes(polyTreeNodes);
  }

  PolygonTreeNode::PolygonTreeNode() : parent(NULL), removed(false) {}
  PolygonTreeNode::PolygonTreeNode(PolygonTreeNode *p, const Polygon &poly) : parent(p), polygon(poly), removed(false) {}
  PolygonTreeNode::~PolygonTreeNode() {
    std::vector<PolygonTreeNode*>::iterator itr = children.begin();

    while(itr != children.end()) {
      delete *itr;
      ++itr;
    }
  }

  PolygonTreeNode* PolygonTreeNode::addChild(const Polygon &polygon) {
    PolygonTreeNode *child = new PolygonTreeNode(this, polygon);

    children.push_back(child);

    return child;
  }

  bool PolygonTreeNode::isRootNode() {
    return parent == NULL;
  }

  bool PolygonTreeNode::isRemoved() {
    return removed;
  }

  Polygon& PolygonTreeNode::getPolygon() {
    return polygon;
  }

  // Like addPolygonTreeNodes, this was implemented iteratively in CSG.js, but we're doing it recursively here.
  // Might be worth revisiting.
  void PolygonTreeNode::splitByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                                         std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                                         std::vector<PolygonTreeNode*> &frontNodes,
                                                         std::vector<PolygonTreeNode*> &backNodes) {

    if(children.size() > 0) {
      std::vector<PolygonTreeNode*>::iterator itr = children.begin();

      while(itr != children.end()) {
        (*itr)->splitByPlane(plane, coplanarFrontNodes, coplanarBackNodes, frontNodes, backNodes);
        ++itr;
      }
    } else {
      splitLeafByPlane(plane, coplanarFrontNodes, coplanarBackNodes, frontNodes, backNodes);
    }
  }

  void PolygonTreeNode::splitLeafByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                                             std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                                             std::vector<PolygonTreeNode*> &frontNodes,
                                                             std::vector<PolygonTreeNode*> &backNodes) {
#ifdef CSGJS_DEBUG
    if(children.size() > 0) {
      throw std::runtime_error("trying to split non-leaf node");
    }
#endif

    std::pair<Vector3, float> bound = polygon.boundingSphere();
    float sphereRadius = bound.second;
    Vector3 sphereCenter = bound.first;

    Vector3 planeNormal = plane.normal;
    float d = planeNormal.dot(sphereCenter) - plane.w;
//    std::cout << d << " (" << sphereRadius << "," << sphereCenter << ") " << polygon << plane << std::endl;
    if(d > sphereRadius) {
      frontNodes.push_back(this);
    } else if(d < -sphereRadius) {
      backNodes.push_back(this);
    } else {
      splitPolygonByPlane(plane, coplanarFrontNodes, coplanarBackNodes, frontNodes, backNodes);
    }
  }

  void PolygonTreeNode::splitPolygonByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                                                std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                                                std::vector<PolygonTreeNode*> &frontNodes,
                                                                std::vector<PolygonTreeNode*> &backNodes) {
    if(plane == polygon.plane) {
      // if the polygon's plane is exactly the same as the cutting plane it as a coplanar front 
      coplanarFrontNodes.push_back(this);
    } else {
      std::vector<bool> vertexIsBack;
      vertexIsBack.reserve(polygon.vertices.size());

      std::vector<Vertex>::iterator itr = polygon.vertices.begin();
      bool hasFront = false;
      bool hasBack = false;
      while(itr != polygon.vertices.end()) {
        float t = plane.normal.dot(itr->pos)-plane.w;
        bool isBack = t < 0;
        vertexIsBack.push_back(isBack);
        if(t > EPS) {
          hasFront = true;
        }
        if(t < NEG_EPS) {
          hasBack = true;
        }
        ++itr;
      }

      if(!hasFront && !hasBack) {
        if(plane.normal.dot(polygon.plane.normal) >= 0) {
          // if the polygon's plane is in the same direction as the cutting plane
          // and all of our vertices were within tolerance of being on the plane
          coplanarFrontNodes.push_back(this);
        } else {
          // if the polygon's plane is in the opposite direction as the cutting plane
          // and all of our vertices were within tolerance of being on the plane
          coplanarBackNodes.push_back(this);
        }
      } else if(!hasBack) {
        // if the polygon only has vertices in front of the cutting plane
        frontNodes.push_back(this);
      } else if(!hasFront) {
        // if the polygon only has vertices behind the cutting plane
        backNodes.push_back(this);
      } else {
        // the polygon crosses the cutting plane and needs to be divided into a front and back polygon

        std::vector<Vertex> frontVertices;
        std::vector<Vertex> backVertices;

        int numVertices = polygon.vertices.size();
        for(int i = 0; i < numVertices; i++) {
          int nextI = i == (numVertices-1) ? 0 : i+1;
          Vertex vertex = polygon.vertices[i];
          Vertex nextVertex = polygon.vertices[nextI];
          bool isBack = vertexIsBack[i];
          bool nextIsBack = vertexIsBack[nextI];
          if(isBack == nextIsBack) {
            // line segment is entirely on one side of the plane
            if(isBack) {
              backVertices.push_back(vertex);
            } else {
              frontVertices.push_back(vertex);
            }
          } else {
            // line segment intersects plane
            Vector3 pos = vertex.pos;
            Vector3 nextPos = nextVertex.pos;
            Vertex intersectionV(plane.splitLineBetweenPoints(pos, nextPos));
            if(isBack) {
              backVertices.push_back(vertex);
              backVertices.push_back(intersectionV);
              frontVertices.push_back(intersectionV);
            } else {
              frontVertices.push_back(vertex);
              frontVertices.push_back(intersectionV);
              backVertices.push_back(intersectionV);
            }
          }
        }

        if(backVertices.size() >= 3) {
          int numBackVertices = backVertices.size();
          Vertex prevVertex = backVertices[numBackVertices-1];
          std::vector<Vertex>::iterator backVertsItr = backVertices.begin();
          while(backVertsItr != backVertices.end()) {
            Vertex v = *backVertsItr;
            if(v.pos.distanceTo(prevVertex.pos) < EPS) {
              backVertsItr = backVertices.erase(backVertsItr);
            } else {
              prevVertex = v;
              ++backVertsItr;
            }
          }
        }

        if(frontVertices.size() >= 3) {
          int numFrontVertices = frontVertices.size();
          Vertex prevVertex = frontVertices[numFrontVertices-1];
          std::vector<Vertex>::iterator frontVertsItr = frontVertices.begin();
          while(frontVertsItr != frontVertices.end()) {
            Vertex v = *frontVertsItr;
            if(v.pos.distanceTo(prevVertex.pos) < EPS) {
              frontVertsItr = frontVertices.erase(frontVertsItr);
            } else {
              prevVertex = v;
              ++frontVertsItr;
            }
          }
        }

        if(frontVertices.size() >= 3) {
          PolygonTreeNode *node = addChild(Polygon(std::move(frontVertices), polygon.plane));
          frontNodes.push_back(node);
        }
        if(backVertices.size() >= 3) {
          PolygonTreeNode *node = addChild(Polygon(std::move(backVertices), polygon.plane));
          backNodes.push_back(node);
        }
      }
    }
  }

  std::ostream& operator<<(std::ostream& os, const Tree &tree) {
//    os << tree.rootnode << std::endl;
    os << tree.polygonTree << std::endl;
    return os;
  }

  std::ostream& indentChildNodes(std::ostream& os, const PolygonTreeNode *node, int level) {
    for(int i = 0; i < level; i++) {
      os << "  ";
    }
    os << node->polygon;

    if(node->children.size() > 0) {
      os << "Children: ";
      os << std::endl;
      std::vector<PolygonTreeNode*>::const_iterator itr = node->children.begin();

      while(itr != node->children.end()) {
        indentChildNodes(os, *itr, level+1);
        ++itr;
      }
    } else {
      os << std::endl;
    }
    return os;
  }

  std::ostream& operator<<(std::ostream& os, const PolygonTreeNode &node) {
    return indentChildNodes(os, &node, 0);
  }
}
