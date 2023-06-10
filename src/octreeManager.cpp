#include "octreeManager.hpp"

#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>
#include <iostream>
#include <random>
#include <stack>
#include <unordered_map>
#include <vector>

namespace etudegl {

TreeNode* OctManager::getChildNodeFromPosition(const TreeNode& parent, NodePosition position) {
    TreeNode* toReturn{nullptr};
    switch (position) {
        case un:
            if (parent.UN) {
                toReturn = parent.UN;
            }
            break;
        case ue:
            if (parent.UE) {
                toReturn = parent.UE;
            }
            break;
        case us:
            if (parent.US) {
                toReturn = parent.US;
            }
            break;
        case uw:
            if (parent.UW) {
                toReturn = parent.UW;
            }
            break;
        case ln:
            if (parent.LN) {
                toReturn = parent.LN;
            }
            break;
        case le:
            if (parent.LE) {
                toReturn = parent.LE;
            }
            break;
        case ls:
            if (parent.LS) {
                toReturn = parent.LS;
            }
            break;
        case lw:
            if (parent.LW) {
                toReturn = parent.LW;
            }
            break;
    }
    return toReturn;
}

void OctManager::createNodeAtPosition(TreeNode* parent, const NodePosition& position,
                                      const std::vector<NodePosition>& path) {
    TreeNode* newNode = new TreeNode();
    newNode->path = path;
    switch (position) {
        case un:
            if (!parent->UN) {
                parent->UN = newNode;
            }
            break;
        case ue:
            if (!parent->UE) {
                parent->UE = newNode;
            }
            break;
        case us:
            if (!parent->US) {
                parent->US = newNode;
            }
            break;
        case uw:
            if (!parent->UW) {
                parent->UW = newNode;
            }
            break;
        case ln:
            if (!parent->LN) {
                parent->LN = newNode;
            }
            break;
        case le:
            if (!parent->LE) {
                parent->LE = newNode;
            }
            break;
        case ls:
            if (!parent->LS) {
                parent->LS = newNode;
            }
            break;
        case lw:
            if (!parent->LW) {
                parent->LW = newNode;
            }
            break;
    }
}

bool OctManager::isLeafNode(const TreeNode& node) {
    if (node.UN || node.UE || node.US || node.UW || node.LN || node.LE || node.LS || node.UW) {
        return false;
    }

    return true;
}

void OctManager::addNode(const std::vector<NodePosition>& path) {
    TreeNode* currentNode = &tree;

    for (auto position : path) {
        if (getChildNodeFromPosition(*currentNode, position)) {
            currentNode = getChildNodeFromPosition(*currentNode, position);
        } else {
            createNodeAtPosition(currentNode, position, path);
            currentNode = getChildNodeFromPosition(*currentNode, position);
        }
    }
}

void OctManager::printFullTree(const TreeNode& node) {
    NodePosition positions[8]{un, ue, us, uw, ln, le, ls, lw};

    for (auto pos : positions) {
        std::cout << "Node at " << pos << ": ";
        if (getChildNodeFromPosition(node, pos)) {
            std::cout << "treenode"
                      << "\n";
            TreeNode childNode = *getChildNodeFromPosition(node, pos);
            std::cout << "ENTERING CHILD NODE"
                      << "\n";
            printFullTree(childNode);
            std::cout << "EXITING CHILD NODE"
                      << "\n";
        } else {
            std::cout << "empty"
                      << "\n";
        }
    }
}

void OctManager::init() {
    // printFullTree(tree);
}

/**
 * @brief cube origin is the vertex of the cube in with the smallest x, y and z
 * coords
 *
 * @param path path to cube in oct tree
 * @param maxdepth maximum index of path
 * @param maxlength length of the cube if root node is leaf
 */
glm::vec3 pathToCubeOrigin(const std::vector<NodePosition>& path, const uint& maxdepth, const uint& maxlength) {
    if (path.empty()) {
        return glm::vec3{0.0f};
    }

    uint currentDepth = 1;  // at depth 0 is the root node
    glm::vec3 origin{0.0f, 0.0f, 0.0f};

    // the position of the origin of the cube is dependant on it's position in the oct tree
    for (NodePosition position : path) {
        switch (position) {
            case un:
                origin += glm::vec3{0.0f, 0.0f, (float)maxlength} / (2.0f * currentDepth);
                break;
            case ue:
                origin += glm::vec3{0.0f, (float)maxlength, (float)maxlength} / (2.0f * currentDepth);
                break;
            case us:
                origin += glm::vec3{(float)maxlength, (float)maxlength, (float)maxlength} / (2.0f * currentDepth);
                break;
            case uw:
                origin += glm::vec3{(float)maxlength, 0.0f, (float)maxlength} / (2.0f * currentDepth);
                break;
            case ln:
                origin += glm::vec3{0.0f, 0.0f, 0.0f} / (2.0f * currentDepth);
                break;
            case le:
                origin += glm::vec3{0.0f, (float)maxlength, 0.0f} / (2.0f * currentDepth);
                break;
            case ls:
                origin += glm::vec3{(float)maxlength, (float)maxlength, 0.0f} / (2.0f * currentDepth);
                break;
            case lw:
                origin += glm::vec3{(float)maxlength, 0.0f, 0.0f} / (2.0f * currentDepth);
                break;
        }
    }

    return origin;
}

std::vector<glm::vec<3, float>> getCubeLocalVertices(const std::vector<NodePosition> path, const uint& maxdepth,
                                                     const uint maxlength) {
    std::vector<glm::vec3> vertices{};
    float length = (float)maxlength / (2.0f * path.size());

    //    4--------6
    //   /|       /|
    //  / |      / |
    // 5--------7  |
    // |  0-----|--2
    // | /      | /
    // |/       |/
    // 1--------3

    vertices.push_back(glm::vec3{0.f});                     // origin
    vertices.push_back(glm::vec3{length, 0.f, 0.f});        // 1
    vertices.push_back(glm::vec3{0.f, length, 0.f});        // 2
    vertices.push_back(glm::vec3{length, length, 0.f});     // 3
    vertices.push_back(glm::vec3{0.f, 0.f, length});        // 4
    vertices.push_back(glm::vec3{length, 0.f, length});     // 5
    vertices.push_back(glm::vec3{0.f, length, length});     // 6
    vertices.push_back(glm::vec3{length, length, length});  // 7

    return vertices;
}

void OctManager::generateTreeMesh(std::vector<float>& vertices, std::vector<uint>& indices) {
    std::unordered_map<glm::vec3, uint> vertexTable{};

    std::stack<TreeNode*> nodeStack{};
    nodeStack.push(&tree);

    while (!nodeStack.empty()) {
        TreeNode* node = nodeStack.top();
        nodeStack.pop();

        if (isLeafNode(*node)) {
            glm::vec3 nodeOrigin = pathToCubeOrigin(node->path, maxdepth, rootNodeEdgeLength);

            // generate triangles and indices

            std::vector<glm::vec3> localVertices = getCubeLocalVertices(node->path, maxdepth, rootNodeEdgeLength);

            for (auto& vertex : localVertices) {
                vertex += nodeOrigin;
            }

            std::vector<uint> baseIndices{0, 1, 2, 2, 1, 3, 0, 4, 5, 0, 5, 1, 1, 5, 7, 1, 7, 3,
                                          2, 3, 7, 2, 7, 6, 2, 6, 4, 0, 6, 4, 5, 4, 7, 7, 4, 6};

            // no duplicate vertices
            for (uint& index : baseIndices) {
                if (vertexTable.count(localVertices[index]) > 0) {  // is in hashtable
                    index = vertexTable[localVertices[index]];
                } else {
                    vertices.push_back(localVertices[index].x);
                    vertices.push_back(localVertices[index].y);
                    vertices.push_back(localVertices[index].z);
                    vertexTable[localVertices[index]] =
                        vertices.size() / 3 - 1;      // add to hashtable with true index value
                    index = vertices.size() / 3 - 1;  // change index to be the true index
                }
            }

            indices.insert(indices.end(), baseIndices.begin(), baseIndices.end());  // concat with index vector

            continue;  // leaf node = no children
        }

        // push node children on stack
        if (node->LW != nullptr) {
            nodeStack.push(node->LW);
        }
        if (node->LS != nullptr) {
            nodeStack.push(node->LS);
        }
        if (node->LE != nullptr) {
            nodeStack.push(node->LE);
        }
        if (node->LN != nullptr) {
            nodeStack.push(node->LN);
        }
        if (node->UW != nullptr) {
            nodeStack.push(node->UW);
        }
        if (node->US != nullptr) {
            nodeStack.push(node->US);
        }
        if (node->UE != nullptr) {
            nodeStack.push(node->UE);
        }
        if (node->UN != nullptr) {
            nodeStack.push(node->UN);
        }
    }
}

}  // namespace etudegl