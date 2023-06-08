#include "engine.hpp"

int main() {
    etudegl::Engine engine{};

    engine.init();

    engine.run();

    engine.cleanup();

    return 0;
}