#include "engine.hpp"

namespace etudegl {

void Engine::init() {
    int status = renderer.init();
    if (status != 0) {
        std::cout << "Something went wrong initialising the Engine."
                  << "\n";
    }

    scene.init();

    // adding some nodes in the tree
    std::vector<NodePosition> path = {ln, le};
    scene.octManager.addNode(path);
    std::vector<NodePosition> path2 = {le};
    scene.octManager.addNode(path2);
    std::vector<NodePosition> path3 = {ls};
    scene.octManager.addNode(path3);
    std::vector<NodePosition> path4 = {ln};
    scene.octManager.addNode(path4);
    std::vector<NodePosition> path5 = {lw};
    scene.octManager.addNode(path5);
    // std::vector<NodePosition> path4 = {us, ls, le}; 
    // scene.octManager.addNode(path4);
    // std::vector<NodePosition> path5 = {us, us, ls}; 
    // scene.octManager.addNode(path5);

    // generate mesh
    std::vector<float> vertices{};
    std::vector<uint> indices{};
    scene.octManager.generateTreeMesh(vertices, indices);

    renderer.createBuffers(vertices, indices);
}

void Engine::run() {
    bool quitCall = false;
    while (!quitCall) {
        quitCall = renderer.run();
    }
}

void Engine::cleanup() { renderer.cleanup(); }

}  // namespace etudegl