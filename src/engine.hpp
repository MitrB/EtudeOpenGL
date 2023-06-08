#pragma once

#include "renderer.hpp"

namespace etudegl {
class Engine {
public:
    ~Engine() { cleanup(); }

    Renderer renderer;
    void init();
    void run();  // GameLoop
    void cleanup();
};

}  // namespace etudegl