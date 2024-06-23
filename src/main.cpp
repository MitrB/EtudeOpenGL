#include "engine.hpp"


int main() {

    Engine::get_instance().init();

    Engine::get_instance().run();

    return 0;
}