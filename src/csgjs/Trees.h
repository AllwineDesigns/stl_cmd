#ifndef __CSGJS_TREES__
#define __CSGJS_TREES__

#include "csgjs/util.h"
#include <vector>
#include "csgjs/math/Plane.h"
#include "csgjs/math/Polygon3.h"

namespace csgjs {

  class PolygonTreeNode {
    private:
      PolygonTreeNode *parent;
      std::vector<PolygonTreeNode*> children;
      Polygon polygon;
      bool removed;

      void splitLeafByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                         std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                         std::vector<PolygonTreeNode*> &frontNodes,
                                         std::vector<PolygonTreeNode*> &backNodes);

      void splitPolygonByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                                   std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                                   std::vector<PolygonTreeNode*> &frontNodes,
                                                   std::vector<PolygonTreeNode*> &backNodes);

    public:
      PolygonTreeNode();
      PolygonTreeNode(const Polygon &polygon);
      PolygonTreeNode(PolygonTreeNode *parent, const Polygon &polygon);
      ~PolygonTreeNode();

      PolygonTreeNode* addChild(const Polygon &polygon);

      bool isRootNode() const;
      bool isRemoved() const;
      int countNodes() const;
      Polygon& getPolygon();
      void splitByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes, 
                                            std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                            std::vector<PolygonTreeNode*> &frontNodes,
                                            std::vector<PolygonTreeNode*> &backNodes);

      friend std::ostream& indentChildNodes(std::ostream& os, const PolygonTreeNode *node, int level);
      friend std::ostream& operator<<(std::ostream& os, const PolygonTreeNode &polygonTreeNode);
  };


  class Node {
    private:
      Plane plane;
      Node* front;
      Node* back;
      Node* parent;
      std::vector<PolygonTreeNode*> polygonTreeNodes;

    public:
      Node();
      Node(Node* p);

      bool isRootNode() const;
      void addPolygonTreeNodes(const std::vector<PolygonTreeNode*> &polyTreeNodes);
  };

  // Root node of the CSG tree and PolygonTree
  class Tree {
    private:
      Node rootnode;
      PolygonTreeNode polygonTree;

    public:
      Tree(const std::vector<Polygon> &polygons);

      friend std::ostream& operator<<(std::ostream& os, const Tree &tree);
  };
}

#endif
