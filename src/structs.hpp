#pragma once

#include <cstdint>
#include <functional>
#include <glm/glm.hpp>
#include <glm/fwd.hpp>
#include <math.h>
#include <glm/gtx/hash.hpp>
#include <vector>
#include <string>


struct PhysicsBody {
    glm::vec3 position;
    glm::vec3 velocity;
};


struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 texture_coords{};

    bool operator==(const Vertex& other) const { return position == other.position && normal == other.normal; }
};

// from: https://stackoverflow.com/a/57595105
template <typename T, typename... Rest>
void hashCombine(std::size_t& seed, T const& v, Rest&&... rest) {
    std::hash<T> hasher;
    seed ^= (hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2));
    int i[] = {0, (hashCombine(seed, std::forward<Rest>(rest)), 0)...};
    (void)(i);
}

namespace std {
template <>
struct hash<Vertex> {
    size_t operator()(Vertex const& vertex) const {
        size_t seed = 0;
        hashCombine(seed, vertex.position, vertex.normal);
        return seed;
    }
};
}  // namespace std

struct Texture {
    unsigned int id;
    std::string type;
};

struct Mesh {
    std::vector<uint16_t> indices; 
    std::vector<Vertex> vertices;
    std::vector<Texture> textures;

    unsigned int VAO, VBO, EBO;
};