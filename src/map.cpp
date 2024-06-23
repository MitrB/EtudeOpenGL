#include "map.hpp"
#include <array>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <ostream>
#include <vector>
#include "ecs.hpp"
#include "engine.hpp"
#include "structs.hpp"

Structure Map::create_structure(Mesh mesh) {
    Entity entity = Engine::get_instance().coordinator.create_entity();

    Structure structure{};
    structure.entity = entity;
    structure.structure_type = 1;
    Engine::get_instance().coordinator.add_component(entity, mesh);

    CollisionShapeCube cube{};
    cube.length = 2;
    Engine::get_instance().coordinator.add_component(entity, cube);

    return structure;
}


void Map::generate_grid() {
    std::cout << "generating grid" << "\n";

    Mesh mesh{};
    ModelManager::load_mesh("../assets/cube_textured.obj", mesh);

    Texture texture{};
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../assets/wall.jpg", &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture" << "\n";
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    unsigned int texture_id;
    glGenTextures(1, &texture_id);  
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    texture.id = texture_id - 1;
    mesh.textures.push_back(texture);

    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);

    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint16_t), mesh.indices.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coords));
    glBindVertexArray(0);


    for (int i=0; i<10; i++) {
        for (int j=0; j<10; j++) {
            Structure structure = create_structure(mesh);
            place_structure(i, 0, j, structure);
        }
    }
    Structure structure = create_structure(mesh);
    place_structure(4, 2, 4, structure);
    structure = create_structure(mesh);
    place_structure(2, 1, 4, structure);
    structure = create_structure(mesh);
    place_structure(3, 1, 4, structure);
    structure = create_structure(mesh);
    place_structure(3, 2, 4, structure);
    structure = create_structure(mesh);
    place_structure(3, 3, 4, structure);
    structure = create_structure(mesh);
    place_structure(1, 1, 4, structure);
    structure = create_structure(mesh);
    place_structure(1, 1, 3, structure);

    structure = create_structure(mesh);

}

void Map::destroy_structure(Structure structure) {
    Engine::get_instance().coordinator.destroy_entity(structure.entity);
}

void Map::place_structure(unsigned int x, unsigned int y, unsigned int z, Structure structure) {
    if (_grid.grid[x][y][z].entity != 0) {
        destroy_structure(_grid.grid[x][y][z]);
    }
    Transformation transform{};
    transform.position.x = x*structure_size;
    transform.position.y = y*structure_size;
    transform.position.z = z*structure_size;
    Engine::get_instance().coordinator.add_component(structure.entity, transform);
    _grid.grid[x][y][z] = structure;
}

Structure Map::get_structure_at_position(glm::vec3 position) {
    int x_floor = static_cast<int>((position.x + 1.0)/structure_size);
    int y_floor = static_cast<int>((position.y + 1.0)/structure_size);
    int z_floor = static_cast<int>((position.z + 1.0)/structure_size);
    std::cout << "-----" << "\n";
    std::cout << x_floor << "\n";
    std::cout << y_floor << "\n";
    std::cout << z_floor << "\n";
    if (x_floor < 0 or y_floor < 0 or z_floor < 0 or x_floor > GRID_DIMENSION or y_floor > GRID_DIMENSION or z_floor > GRID_DIMENSION) {
        return Structure{0, 0};
    }

    Structure structure = _grid.grid[x_floor][y_floor][z_floor];
    return structure;
}

unsigned int Map::get_structure_type_at_position(glm::vec3 position) {
    Structure structure = get_structure_at_position(position);
    unsigned int type = structure.structure_type;
    std::cout << type << "\n";
    return type;
}

bool Map::is_entity_on_ground(Entity entity, Coordinator& coordinator) {
    CollisionShapeCube entity_cube = coordinator.get_component<CollisionShapeCube>(entity);
    Transformation& entity_position = coordinator.get_component<Transformation>(entity);
    glm::vec3 entity_cube_position = entity_cube.offset + entity_position.position;

    std::vector<Structure> structures{};
    float offset = entity_cube.length/2;
    std::vector<glm::vec3> offsets{
        glm::vec3{-offset + 0.1f, -offset - 0.001f, -offset + 0.1f},
        glm::vec3{offset - 0.1f, -offset - 0.001f, -offset + 0.1f},
        glm::vec3{-offset + 0.1f, -offset - 0.001f, offset - 0.1f},
        glm::vec3{offset- 0.1f, -offset - 0.001f, offset - 0.1f},
    };

    for (glm::vec3 pos : offsets) {
        Structure structure = get_structure_at_position(entity_cube_position + pos);
        if (structure.structure_type !=0) {
            structures.push_back(structure);
        }
    }

    if (structures.size() > 0) {
        std::cout << "entity on ground" << "\n";
        return true;
    }
    else {
        std::cout << "entity on not on ground" << "\n";
        return false;
    }
}


void Map::solve_collision(Entity entity, Coordinator& coordinator) {
    CollisionShapeCube entity_cube = coordinator.get_component<CollisionShapeCube>(entity);
    Transformation& entity_position = coordinator.get_component<Transformation>(entity);

    glm::vec3 entity_cube_position = entity_cube.offset + entity_position.position;

    std::cout << "------" << "\n";
    std::cout << entity_cube_position.x << "\n";
    std::cout << entity_cube_position.y << "\n";
    std::cout << entity_cube_position.z << "\n";

    std::vector<Structure> structures{};
    float offset = entity_cube.length/2;
    std::vector<glm::vec3> offsets{
        glm::vec3{-offset, -offset, -offset},
        glm::vec3{offset, -offset, -offset},
        glm::vec3{-offset, -offset, offset},
        glm::vec3{offset, -offset, offset},
        glm::vec3{-offset, offset, -offset},
        glm::vec3{offset, offset, -offset},
        glm::vec3{-offset,offset, offset},
        glm::vec3{offset, offset, offset},
    };

    for (glm::vec3 pos : offsets) {
        Structure structure = get_structure_at_position(entity_cube_position + pos);
        if (structure.structure_type !=0) {
            structures.push_back(structure);
        }
    }

    std::cout << "Collisions: " << structures.size() << "\n";

    for (Structure structure : structures) {
        CollisionShapeCube structure_cube = Engine::get_instance().coordinator.get_component<CollisionShapeCube>(structure.entity);
        Transformation structure_position = Engine::get_instance().coordinator.get_component<Transformation>(structure.entity);

        float dx = std::min(structure_position.position.x + 1.0 - entity_position.position.x, entity_position.position.x + 0.5 - structure_position.position.x);
        float dy = std::min(structure_position.position.y + 1.0 - entity_position.position.y, entity_position.position.y + 0.5 - structure_position.position.y);
        float dz = std::min(structure_position.position.z + 1.0 - entity_position.position.z, entity_position.position.z + 0.5 - structure_position.position.z);
        float smallest_change = std::min(dx, std::min(dy, dz));
        if (smallest_change == dx) {
            if (entity_position.position.x >= structure_position.position.x) {
                entity_position.position.x = structure_position.position.x + 1.0 + entity_cube.length/2;
            }
            else {
                entity_position.position.x = structure_position.position.x - 1.0 - entity_cube.length/2;
            }
        }
        else if (smallest_change == dy) {
            if (entity_position.position.y > structure_position.position.y) {
                entity_position.position.y = structure_position.position.y + 1.0 + entity_cube.length/2;
            }
            else {
                entity_position.position.y = structure_position.position.y - 1.0 - entity_cube.length/2;
            }
        }
        else {
            if (entity_position.position.z >= structure_position.position.z) {
                entity_position.position.z = structure_position.position.z + 1.0 + entity_cube.length/2;
            }
            else {
                entity_position.position.z = structure_position.position.z - 1.0 - entity_cube.length/2;
            }
        }
    }





    // collision response
}