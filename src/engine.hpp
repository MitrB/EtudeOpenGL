#pragma once

#include <memory>
#include <vector>
#include "ecs.h"
#include "ecs.h"
#include "physics.hpp"
#include "renderer.hpp"
#include "structs.hpp"

class Engine {
    public:
        void init();
        void run();
        


    private:
        bool is_running{true};
        // systems
        std::shared_ptr<Renderer> renderer_system;
        std::shared_ptr<Physics> physics_system;

};