#include "engine.hpp"
#include <GLFW/glfw3.h>
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <random>
#include "ecs.h"
#include "model_manager.hpp"
#include "structs.hpp"

Coordinator coordinator;

float random_float(float min, float max, std::mt19937 &gen) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

void Engine::init() {
    coordinator.init();

    coordinator.register_component<PhysicsBody>();
    coordinator.register_component<Mesh>();

    physics_system = coordinator.register_system<Physics>();
    physics_system->coordinator = &coordinator;
    Signature signature_physics{};
    signature_physics.set(coordinator.get_component_type<PhysicsBody>(), true);
    coordinator.set_system_signature<Physics>(signature_physics);

    renderer_system = coordinator.register_system<Renderer>();
    renderer_system->coordinator = &coordinator;
    Signature signature_renderer{};
    signature_renderer.set(coordinator.get_component_type<PhysicsBody>(), true);
    signature_renderer.set(coordinator.get_component_type<Mesh>(), true);
    coordinator.set_system_signature<Physics>(signature_renderer);

    std::random_device rd;
    std::mt19937 gen(rd());

    Texture texture{};
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../assets/wall.jpg", &width, &height, &nrChannels, 0);
    if (!data) {
        std::cout << "Failed to load texture" << "\n";
    }
    unsigned int texture_id;
    glGenTextures(1, &texture_id);  
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    for (size_t i; i < 1000; i++) {
        Entity e = coordinator.create_entity();
        PhysicsBody body{};
        body.position.x = random_float(-100, 100, gen);
        body.position.y = random_float(-100, 100, gen);
        body.position.z = random_float(-100, 100, gen);
        coordinator.add_component(e, body);

        Mesh mesh{};
        ModelManager::load_mesh("../assets/cube_textured.obj", mesh);

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

        coordinator.add_component(e, mesh);

        // glGenVertexArrays(1, &lightVAO);
        // glBindVertexArray(lightVAO);
        // glBindBuffer(GL_ARRAY_BUFFER, VBO);

        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        // glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(uint16_t), model.indices.data(), GL_STATIC_DRAW);

        // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
        // glEnableVertexAttribArray(0);
        // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        // glEnableVertexAttribArray(1);
    }
}

void Engine::run() {
    float delta = 0.0f;
    float last_frame = glfwGetTime();
    float frame_time = 1.0f/60.0f;
    float delta_sum = 0.0f;

    int frames_count = 0;
    int fps = 0;
    float second_count = 0.0f;


    while (!renderer_system->close_window()) {
        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;
        delta_sum += delta;


        Update update;

        while(delta_sum >= frame_time) {
            delta_sum -= frame_time;
            update.delta = frame_time;

            physics_system->update(update);
        }

        update.delta = delta;
        renderer_system->update(update);

        // frames
        frames_count += 1;
        second_count += delta;
        if (second_count >= 1.0f) {
            fps = frames_count / second_count;
            frames_count = 0;
            second_count = 0;
            std::cout << fps << "\n";
        }
    }
}
