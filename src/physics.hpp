#pragma once
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include "ecs.h"
#include "structs.hpp"

class Physics : public System {
    public:
        void update(Update update) {
            for (Entity e1 : entities) {
                PhysicsBody& e1_info = update.coordinator->get_component<PhysicsBody>(e1);
                glm::vec3 force{0.0f, 0.0f, 0.0f};
                for (Entity e2 : entities) {
                    if (e1 != e2) {
                        PhysicsBody& e2_info = update.coordinator->get_component<PhysicsBody>(e2);
                        float distance = std::pow((e1_info.position.x - e2_info.position.x), 2.0f) + std::pow((e1_info.position.y - e2_info.position.y), 2.0f) + std::pow((e1_info.position.z - e2_info.position.z), 2.0f);
                        glm::vec3 direction = glm::normalize(e2_info.position - e1_info.position);
                        if (distance < 0.05f) {
                            force = -direction*glm::length(e1_info.velocity + e2_info.velocity)/2.0f;
                            break;
                        }
                        force += direction / (distance);
                        
                    }
                }
                e1_info.velocity += force * update.delta * 0.01f;
                e1_info.position += e1_info.velocity;
            } 
        }

};