#include "engine.hpp"


void Engine::init() {
    renderer = Renderer();
    renderer.init();
}

void Engine::run() {
    renderer.draw();
}