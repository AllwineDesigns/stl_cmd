#include "csgjs/Trees.h"
#include <list>

namespace csgjs {
  Node::Node() : parent(NULL), front(NULL), back(NULL)  {
  }

  Node::Node(Node *p) : parent(p), front(NULL), back(NULL) {
  }

  void Node::invert() {
    plane = plane.flipped();

    if(front != NULL) {
      front->invert();
    }

    if(back != NULL) {
      back->invert();
    }

    Node *n = front;
    front = back;
    back = n;
  }

  void Node::addPolygonTreeNodes(const std::vector<PolygonTreeNode*> &polyTreeNodes) {
    std::vector<PolygonTreeNode*> frontNodes;
    std::vector<PolygonTreeNode*> backNodes;

    if(polyTreeNodes.size() > 0) {
      int pick = fastRandom(polyTreeNodes.size());
  //    int pick = 0;

      plane = polyTreeNodes[pick]->getPolygon().plane;
    }

    std::vector<PolygonTreeNode*>::const_iterator itr = polyTreeNodes.begin();
    while(itr != polyTreeNodes.end()) {
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

  void Node::clipTo(Tree &tree, bool alsoRemoveCoplanarFront) {
    if(polygonTreeNodes.size() > 0) {
      tree.rootnode.clipPolygons(polygonTreeNodes, alsoRemoveCoplanarFront);
    }
    if(front != NULL) {
      front->clipTo(tree, alsoRemoveCoplanarFront);
    }
    if(back != NULL) {
      back->clipTo(tree, alsoRemoveCoplanarFront);
    }
  }

  // Returns true if any triangles exist on the front side of the triangle
  //
  // breadth first search for provided plane, then check if any front nodes exist
  bool Node::hasFrontNodes(const Plane &p) const {
    std::list<const Node*> queue;

    queue.push_back(this);

    while(!queue.empty()) {
      const Node* curNode = queue.front();
      queue.pop_front();

      if(curNode->plane.isEqualWithinTolerance(p)) {
        if(curNode->front != NULL) {
          return true;
        }
      } else {
        if(curNode->front != NULL) {
          queue.push_back(curNode->front);
        }
        if(curNode->back != NULL) {
          queue.push_back(curNode->back);
        }
      }
    }
    return false;
  }

  void Node::clipPolygons(std::vector<PolygonTreeNode*> &polyTreeNodes, bool alsoRemoveCoplanarFront) {
    std::vector<PolygonTreeNode*> frontNodes;
    std::vector<PolygonTreeNode*> backNodes;

    std::vector<PolygonTreeNode*>::iterator itr = polyTreeNodes.begin();

    while(itr != polyTreeNodes.end()) {
      PolygonTreeNode *node = (*itr);
      if(!node->isRemoved()) {
        node->splitByPlane(plane, alsoRemoveCoplanarFront ? backNodes : frontNodes, backNodes, frontNodes, backNodes);
      }
      ++itr;
    }

    if(front != NULL && frontNodes.size() > 0) {
      front->clipPolygons(frontNodes);
    }

    if(back != NULL && backNodes.size() > 0) {
      back->clipPolygons(backNodes);
    } else {
      std::vector<PolygonTreeNode*>::iterator backItr = backNodes.begin();

      while(backItr != backNodes.end()) {
        (*backItr)->remove();
        ++backItr;
      }
    }
  }

  Tree::Tree(const std::vector<Polygon> &polygons) {
    addPolygons(polygons);
  }

  bool Tree::hasPolygonsInFront(const Plane &p) const {
    return rootnode.hasFrontNodes(p);
  }

  void Tree::addPolygons(const std::vector<Polygon> &polygons) {
    std::vector<Polygon>::const_iterator itr = polygons.begin();

    std::vector<PolygonTreeNode*> polyTreeNodes;
    polyTreeNodes.reserve(polygons.size());

    while(itr != polygons.end()) {
      polyTreeNodes.push_back(polygonTree.addChild(*itr));
      ++itr;
    }

    rootnode.addPolygonTreeNodes(polyTreeNodes);
  }

  void Tree::invert() {
    polygonTree.invert();
    rootnode.invert();
  }

  void Tree::clipTo(Tree &tree, bool alsoRemoveCoplanarFront) {
    rootnode.clipTo(tree, alsoRemoveCoplanarFront);
  }

  std::vector<Polygon> Tree::toPolygons() {
    std::vector<Polygon> polygons;

    polygonTree.getPolygons(polygons);

    return polygons;
  }

  PolygonTreeNode::PolygonTreeNode() : parent(NULL), removed(false), valid(false) {}
  PolygonTreeNode::PolygonTreeNode(PolygonTreeNode *p, const Polygon &poly) : parent(p), polygon(poly), removed(false), valid(true) {}
  PolygonTreeNode::~PolygonTreeNode() {
    std::vector<PolygonTreeNode*>::iterator itr = children.begin();

    while(itr != children.end()) {
      delete *itr;
      ++itr;
    }
  }

  void PolygonTreeNode::invalidate() {
    valid = false;

    if(parent != NULL) {
      parent->invalidate();
    }
  }

  void PolygonTreeNode::remove() {
#ifdef CSGJS_DEBUG
    if(isRootNode()) {
      throw std::runtime_error("trying to delete root node");
    }
    if(children.size() > 0) {
      throw std::runtime_error("trying to delete node with children");
    }
#endif

    invalidate();

    // Can't delete this without removing pointers in BSP Node objects as well, if we remove from parent's children vector, we'll be creating a memory leak
    // Maybe use some kind of smart pointer so we can remove them here
    //parent->children.erase(std::remove(parent->children.begin(), parent->children.end(), this), parent->children.end());
    //delete this;

  }

  PolygonTreeNode* PolygonTreeNode::addChild(const Polygon &polygon) {
    PolygonTreeNode *child = new PolygonTreeNode(this, polygon);

    children.push_back(child);

    return child;
  }

  bool PolygonTreeNode::isRootNode() const {
    return parent == NULL;
  }

  bool PolygonTreeNode::isRemoved() const {
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
      if(valid) {
        splitLeafByPlane(plane, coplanarFrontNodes, coplanarBackNodes, frontNodes, backNodes);
      }
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

    std::pair<Vector3, csgjs_real> bound = polygon.boundingSphere();
    csgjs_real sphereRadius = bound.second;
    Vector3 sphereCenter = bound.first;

    Vector3 planeNormal = plane.normal;
    csgjs_real d = planeNormal.dot(sphereCenter) - plane.w;
    if(d > sphereRadius) {
      frontNodes.push_back(this);
    } else if(d < -sphereRadius) {
      backNodes.push_back(this);
    } else {
      splitPolygonByPlane(plane, coplanarFrontNodes, coplanarBackNodes, frontNodes, backNodes);
    }
  }

  void PolygonTreeNode::invertRecurse() {
    if(valid) {
      polygon = polygon.flipped();
    }
    std::vector<PolygonTreeNode*>::iterator itr = children.begin();
    while(itr != children.end()) {
      (*itr)->invertRecurse();
      ++itr;
    }
  }

  void PolygonTreeNode::invert() {
#ifdef CSGJS_DEBUG
    if(!isRootNode()) {
      throw std::runtime_error("can only call invert on root node");
    }
#endif
    invertRecurse();
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
        csgjs_real t = plane.normal.dot(itr->pos)-plane.w;
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

  void PolygonTreeNode::getPolygons(std::vector<Polygon> &polygons) const {
    if(valid) {
      polygons.push_back(polygon);
    } else {
      std::vector<PolygonTreeNode*>::const_iterator itr = children.begin();
      while(itr != children.end()) {
        (*itr)->getPolygons(polygons);
        ++itr;
      }
    }
  }

  int PolygonTreeNode::countNodes() const {
    int count = 1;

    std::vector<PolygonTreeNode*>::const_iterator itr = children.begin();
    while(itr != children.end()) {
      count += (*itr)->countNodes();
      ++itr;
    }
    return count;
  }

  std::ostream& operator<<(std::ostream& os, const Tree &tree) {
//    os << tree.rootnode << std::endl;
    os << tree.polygonTree.countNodes() << std::endl;
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
