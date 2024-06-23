#pragma once

#include "ecs.hpp"
#include "physics.hpp"
#include "renderer.hpp"
#include "structs.hpp"
#include "model_manager.hpp"
#include "map.hpp"

#include <array>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <random>


class Engine {
    public:
        static Engine& get_instance() {
            static Engine engine;
            return engine;
        }

        Coordinator coordinator{};
        
        void init();
        void run();
        


    private:
        Entity player;
        Map map{};
        bool is_running{true};
        // systems
        std::shared_ptr<Renderer> renderer_system;
        std::shared_ptr<Physics> physics_system;

};