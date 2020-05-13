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
      bool valid;
      bool removed;

      void splitLeafByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes,
                                         std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                         std::vector<PolygonTreeNode*> &frontNodes,
                                         std::vector<PolygonTreeNode*> &backNodes);

      void splitPolygonByPlane(const Plane &plane, std::vector<PolygonTreeNode*> &coplanarFrontNodes,
                                                   std::vector<PolygonTreeNode*> &coplanarBackNodes,
                                                   std::vector<PolygonTreeNode*> &frontNodes,
                                                   std::vector<PolygonTreeNode*> &backNodes);

      void invertRecurse();

    public:
      PolygonTreeNode();
      PolygonTreeNode(const Polygon &polygon);
      PolygonTreeNode(PolygonTreeNode *parent, const Polygon &polygon);
      ~PolygonTreeNode();

      PolygonTreeNode* addChild(const Polygon &polygon);

      void getPolygons(std::vector<Polygon> &polygons) const;
      void invalidate();
      void remove();
      void invert();
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


  class Tree;

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

      bool hasFrontNodes(const Plane &p) const;
      bool isRootNode() const;
      void invert();
      void clipTo(Tree &tree, bool alsoRemoveCoplanarFront=false);
      void clipPolygons(std::vector<PolygonTreeNode*> &polyTreeNodes, bool alsoRemoveCoplanarFront=false);
      void addPolygonTreeNodes(const std::vector<PolygonTreeNode*> &polyTreeNodes);
  };

  // Root node of the CSG tree and PolygonTree
  class Tree {
    private:
      Node rootnode;
      PolygonTreeNode polygonTree;

    public:
      Tree(const std::vector<Polygon> &polygons);

      void addPolygons(const std::vector<Polygon> &polygons);

      bool hasPolygonsInFront(const Plane &p) const;
      void clipTo(Tree &tree, bool alsoRemoveCoplanarFront=false);
      void invert();
      std::vector<Polygon> toPolygons();

      friend std::ostream& operator<<(std::ostream& os, const Tree &tree);
      friend class Node;
  };
}

#endif
