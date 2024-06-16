#include "engine.hpp"
#include <cstddef>
#include <cstdio>
#include <cwchar>
#include <memory>
#include <random>

Coordinator coordinator;

float random_float(float min, float max, std::mt19937 &gen) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

void Engine::init() {
    coordinator.init();

    coordinator.register_component<PhysicsBody>();
    physics_system = coordinator.register_system<Physics>();
    Signature signature_physics{};
    signature_physics.set(coordinator.get_component_type<PhysicsBody>(), true);
    coordinator.set_system_signature<Physics>(signature_physics);

    renderer_system = coordinator.register_system<Renderer>();
    Signature signature_renderer{};
    signature_renderer.set(coordinator.get_component_type<PhysicsBody>(), true);
    coordinator.set_system_signature<Physics>(signature_renderer);

    std::random_device rd;
    std::mt19937 gen(rd());

    for (size_t i; i < 100; i++) {
        Entity e = coordinator.create_entity();
        PhysicsBody body{};
        body.position.x = random_float(-10, 10, gen);
        body.position.y = random_float(-10, 10, gen);
        body.position.z = random_float(-10, 10, gen);
        coordinator.add_component(e, body);
    }

    // Systems
    renderer_system->init();
}

void Engine::run() {
    float delta = 0.0f;
    float last_frame = 0.0f;
    while (!renderer_system->close_window()) {
        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;

        Update update;
        update.coordinator = &coordinator;
        update.delta = delta;

        renderer_system->update(update);
        physics_system->update(update);
    }
}
