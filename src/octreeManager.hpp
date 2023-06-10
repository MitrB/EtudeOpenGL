#pragma once

#include <cmath>
#include <glm/fwd.hpp>
#include <vector>

namespace etudegl {

/*
 enum representing the 3D spatial position of a node respective to it's parent
 withing the oct tree

 UN: Upper North
 LE: Lower EAST
 etc.

 In logic the cube is rotated so that these correspond to the directions
 North, East, South, West
*/
enum NodePosition { un, ue, us, uw, ln, le, ls, lw };

struct TreeNode {
    std::vector<NodePosition> path{};

    TreeNode* UN{};
    TreeNode* UE{};
    TreeNode* US{};
    TreeNode* UW{};

    TreeNode* LN{};
    TreeNode* LE{};
    TreeNode* LS{};
    TreeNode* LW{};
};

class OctManager {
public:
    const unsigned int maxdepth = 4;
    const unsigned int rootNodeEdgeLength = std::pow(2, maxdepth);

    TreeNode tree{};

    void addNode(const std::vector<NodePosition>& path);
    void removeNode(const char& nodePath);

    void generateTreeMesh(std::vector<float> &vertices, std::vector<uint> &indices);

    void printFullTree(const TreeNode& node);

    void init();

private:
    bool isLeafNode(const TreeNode& node);
    TreeNode* getChildNodeFromPosition(const TreeNode& parent, NodePosition position);
    void createNodeAtPosition(TreeNode* parent, const NodePosition& position, const std::vector<NodePosition>& path);
};
}  // namespace etudegl