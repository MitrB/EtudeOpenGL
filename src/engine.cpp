#include "engine.hpp"

namespace etudegl {

void Engine::init() {
    int status = renderer.init();
    if (status != 0) {
        std::cout << "Something went wrong initialising the Engine."
                  << "\n";
    }
}
void Engine::run() {
    bool quitCall = false;
    while (!quitCall) {
        quitCall = renderer.run();
    }
}

void Engine::cleanup() { renderer.cleanup(); }

}  // namespace etudegl