#pragma once
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/geometric.hpp>
#include "ecs.hpp"
#include "structs.hpp"

class Physics : public System {
    public:
        // void update(Update update) {
        //     for (Entity e1 : entities) {
        //         Position& e1_info_pos = coordinator->get_component<Position>(e1);
        //         PhysicsBody& e1_info_phy = coordinator->get_component<PhysicsBody>(e1);
        //         glm::vec3 force{0.0f, 0.0f, 0.0f};
        //         for (Entity e2 : entities) {
        //             if (e1 != e2) {
        //                 Position& e2_info_pos = coordinator->get_component<Position>(e2);
        //                 PhysicsBody& e2_info_phy = coordinator->get_component<PhysicsBody>(e2);
        //                 float distance = ((e1_info_pos.position.x - e2_info_pos.position.x), 2.0f) + std::pow((e1_info_pos.position.y - e2_info_pos.position.y), 2.0f) + std::pow((e1_info_pos.position.z - e2_info_pos.position.z), 2.0f);
        //                 glm::vec3 direction = glm::normalize(e2_info_pos.position - e1_info_pos.position);
        //                 // if (distance < 0.05f) {
        //                 //     force = -direction*glm::length(e1_info_phy.velocity + e2_info_phy.velocity)/2.0f;
        //                 //     break;
        //                 // }
        //                 force += e2_info_phy.mass*e1_info_phy.mass*direction / (distance + glm::vec3(0.00000001));
                        
        //             }
        //         }
        //         e1_info_phy.velocity += force * update.delta;
        //         e1_info_pos.position += e1_info_phy.velocity;
        //     } 
        // }

        void update(Update update){
            for (Entity entity : entities) {
                Transformation& entity_transformation = coordinator->get_component<Transformation>(entity);
                PhysicsBody& entity_physicsbody = coordinator->get_component<PhysicsBody>(entity);

                float target_velocity = -12.0f;
                entity_physicsbody.velocity.y = (target_velocity - entity_physicsbody.velocity.y)*update.delta + entity_physicsbody.velocity.y;
                entity_transformation.position.y += entity_physicsbody.velocity.y * update.delta;
            }
        }

};