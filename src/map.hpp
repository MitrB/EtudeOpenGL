#pragma once

#include <array>
#include "ecs.hpp"
#include "structs.hpp"

class Engine;

#define GRID_DIMENSION 100

struct Structure {
    unsigned int entity{0};
    unsigned int structure_type{0};
};

struct Grid {
    std::array<std::array<std::array<Structure, GRID_DIMENSION>, GRID_DIMENSION>, GRID_DIMENSION> grid{0,0};
    
};

class Map {
    public:
        Structure create_structure(Mesh mesh);
        void destroy_structure(Structure structure);
        void generate_grid();
        void place_structure(unsigned int x, unsigned int y, unsigned int z, Structure structure);
        unsigned int get_structure_type_at_position(glm::vec3 position);
        Structure get_structure_at_position(glm::vec3 position);
        bool is_entity_on_ground(Entity entity, Coordinator& coordinator);

        void solve_collision(Entity entity, Coordinator& coordinator);
    private:
        Grid _grid{};
        float structure_size{2.0f};
};