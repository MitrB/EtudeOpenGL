#pragma once

#include "renderer.hpp"

namespace etudegl {
class Engine {
public:
    Renderer renderer;
    void init();
    void run(); // GameLoop
    void cleanup();
};

}  // namespace etudegl