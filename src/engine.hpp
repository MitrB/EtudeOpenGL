#pragma once

#include "renderer.hpp"
#include "scene.hpp"

namespace etudegl {
class Engine {
public:
    ~Engine() { cleanup(); }

    Renderer renderer{};
    Scene scene{};
    void init();
    void run();  // GameLoop
    void cleanup();
};

}  // namespace etudegl