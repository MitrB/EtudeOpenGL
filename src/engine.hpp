#pragma once

#include "renderer.hpp"

class Engine {
    public:
        void init();
        void run();


    private:
        Renderer renderer;

};