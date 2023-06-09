#include "octreeManager.hpp"

#include <iostream>
#include <random>

namespace etudegl {

void OctManager::printFullTree(const TreeNode& node) {
    NodePosition positions[8]{un, ue, us, uw, ln, le, ls, lw};

    for (auto pos : positions) {
        std::cout << "Node at " << pos << ": ";
        if (getChildNodeFromPosition(node, pos)) {
            std::cout << "treenode"
                      << "\n";
            TreeNode childNode = *getChildNodeFromPosition(node, pos);
            std::cout << "ENTERING CHILD NODE" << "\n"; 
            printFullTree(childNode);
            std::cout << "EXITING CHILD NODE" << "\n"; 
        } else {
            std::cout << "empty"
                      << "\n";
        }
    }
}

TreeNode* OctManager::getChildNodeFromPosition(const TreeNode& parent,
                                               NodePosition position) {
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
        case e:
            break;
    }
    return toReturn;
}

void OctManager::createNodeAtPosition(TreeNode* parent, NodePosition position) {
    TreeNode* newNode = new TreeNode();
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
        case e:
            break;
    }
}

bool OctManager::isLeafNode(TreeNode node) {
    if (!node.UN || !node.UE || !node.US || !node.UW || !node.LN || !node.LE ||
        !node.LS || !node.UW) {
        return false;
    }

    return true;
}

void OctManager::addNode(const NodePosition (&nodePath)[8]) {
    unsigned int currentDepth{0};
    TreeNode* currentNode = &tree;

    for (auto position : nodePath) {
        std::cout << currentDepth << "\n";
        std::cout << position << "\n";
        std::cout << e << "\n";
        if (position == e) {
            break;
        }
        std::cout << "start"
                  << "\n";
        if (getChildNodeFromPosition(*currentNode, position)) {
            std::cout << "if"
                      << "\n";
            currentNode = getChildNodeFromPosition(*currentNode, position);
        } else {
            std::cout << "else"
                      << "\n";
            createNodeAtPosition(currentNode, position);
            currentNode = getChildNodeFromPosition(*currentNode, position);
        }
        currentDepth += 1;
    }
}

void OctManager::init() {
    const NodePosition path[8] = {ln, le, e, e, e, e, e, e};
    addNode(path);
    const NodePosition path2[8] = {us, le, e, e, e, e, e, e};
    addNode(path2);
 
    printFullTree(tree);
}
}  // namespace etudegl