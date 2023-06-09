#pragma once

#include <cmath>
namespace etudegl {

enum NodePosition { un, ue, us, uw, ln, le, ls, lw, e};

struct TreeNode {
    TreeNode* UN;
    TreeNode* UE;
    TreeNode* US;
    TreeNode* UW;

    TreeNode* LN;
    TreeNode* LE;
    TreeNode* LS;
    TreeNode* LW;
};

class OctManager {
public:
    unsigned int maxdepth = 8;
    unsigned int rootNodeEdgeLength = std::pow(2, maxdepth);

    TreeNode tree{};

    void addNode(const NodePosition (&nodePath)[8]);
    void removeNode(const char& nodePath);

    void printFullTree(const TreeNode &node);

    void init();
private:
    bool isLeafNode(TreeNode node);
    TreeNode* getChildNodeFromPosition(const TreeNode &parent, NodePosition position);
    void createNodeAtPosition(TreeNode* parent, NodePosition position);
};
}  // namespace etudegl