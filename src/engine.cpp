#include "engine.hpp"

#include <GLFW/glfw3.h>
#include <algorithm>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_geometric.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include "ecs.hpp"
#include "structs.hpp"


float random_float(float min, float max, std::mt19937 &gen) {
    std::uniform_real_distribution<float> dis(min, max);
    return dis(gen);
}

void Engine::init() {
    coordinator.init();

    coordinator.register_component<Transformation>();
    coordinator.register_component<PhysicsBody>();
    coordinator.register_component<CollisionShapeCube>();
    coordinator.register_component<Mesh>();

    physics_system = coordinator.register_system<Physics>();
    physics_system->coordinator = &coordinator;
    Signature signature_physics{};
    signature_physics.set(coordinator.get_component_type<Transformation>(), true);
    signature_physics.set(coordinator.get_component_type<PhysicsBody>(), true);
    coordinator.set_system_signature<Physics>(signature_physics);

    renderer_system = coordinator.register_system<Renderer>();
    renderer_system->coordinator = &coordinator;
    Signature signature_renderer{};
    signature_renderer.set(coordinator.get_component_type<Transformation>(), true);
    signature_renderer.set(coordinator.get_component_type<Mesh>(), true);
    coordinator.set_system_signature<Renderer>(signature_renderer);

    std::random_device rd;
    std::mt19937 gen(rd());

    Transformation player_position{glm::vec3{2.0, 3.0, 2.0}};
    PhysicsBody player_physicsbody{};
    CollisionShapeCube collision_shape{};
    player = coordinator.create_entity();
    coordinator.add_component(player, player_position);
    coordinator.add_component(player, player_physicsbody);
    coordinator.add_component(player, collision_shape);
    
    // Mesh mesh{};
    // ModelManager::load_mesh("../assets/sphere.obj", mesh);

    // Texture texture{};
    // int width, height, nrChannels;
    // unsigned char *data = stbi_load("../assets/wall.jpg", &width, &height, &nrChannels, 0);
    // if (!data) {
    //     std::cout << "Failed to load texture" << "\n";
    // }
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // unsigned int texture_id;
    // glGenTextures(1, &texture_id);  
    // glBindTexture(GL_TEXTURE_2D, texture_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    // glGenerateMipmap(GL_TEXTURE_2D);

    // texture.id = texture_id - 1;
    // mesh.textures.push_back(texture);

    // glGenVertexArrays(1, &mesh.VAO);
    // glGenBuffers(1, &mesh.VBO);
    // glGenBuffers(1, &mesh.EBO);

    // glBindVertexArray(mesh.VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);

    // glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(Vertex), mesh.vertices.data(), GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(uint16_t), mesh.indices.data(), GL_STATIC_DRAW);

    // glEnableVertexAttribArray(0);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    // glEnableVertexAttribArray(1);
    // glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    // glEnableVertexAttribArray(2);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coords));
    // glBindVertexArray(0);
    

    map.generate_grid();

    // for (size_t i=0; i < 3; i++) {
    //     Entity e = coordinator.create_entity();

    //     coordinator.add_component(e, mesh);

    //     Position position{};
    //     position.position.x = random_float(-10, 10, gen);
    //     position.position.y = random_float(-10, 10, gen);
    //     position.position.z = random_float(-10, 10, gen);
    //     coordinator.add_component(e, position);

    //     PhysicsBody body{};
    //     coordinator.add_component(e, body);

    // }
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
        glfwPollEvents();
        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;
        delta_sum += delta;

        Update update;

        while(delta_sum >= frame_time) {
            delta_sum -= frame_time;
            update.delta = frame_time;
            glm::vec3& player_position = coordinator.get_component<Transformation>(player).position;
            PhysicsBody& player_body = coordinator.get_component<PhysicsBody>(player);

            GLFWwindow* window = renderer_system->get_window();

            if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
                player_body.velocity.y = 10.0f;

            glm::vec3 camera_front = renderer_system->get_camera_front();
            glm::vec3 camera_up = renderer_system->get_camera_up();
            const float cameraSpeed = 8.0f; 
            glm::vec3 projected_camera_front = glm::normalize(glm::vec3{camera_front.x, 0.0f, camera_front.z});
            glm::vec3 input_direction{0.0f, 0.0f, 0.0f};
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
                input_direction.x += projected_camera_front.x; 
                input_direction.z += projected_camera_front.z; 
            } 
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
                input_direction.x += -projected_camera_front.x;
                input_direction.z += -projected_camera_front.z;
            }
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
                glm::vec3 calculated_velocity = glm::normalize(glm::cross(projected_camera_front, camera_up));
                input_direction.x += -calculated_velocity.x;
                input_direction.z += -calculated_velocity.z;
            }
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
                glm::vec3 calculated_velocity = glm::normalize(glm::cross(projected_camera_front, camera_up));
                input_direction.x += calculated_velocity.x;
                input_direction.z += calculated_velocity.z;
            }

            if (input_direction != glm::vec3(0.0f)) {
                player_body.velocity.x = glm::normalize(input_direction).x;
                player_body.velocity.z = glm::normalize(input_direction).z;
                player_position.x += player_body.velocity.x * update.delta * cameraSpeed;
                player_position.z += player_body.velocity.z * update.delta * cameraSpeed;
            }
            player_position.y += player_body.velocity.y * update.delta;

            PhysicsBody& player_physics = coordinator.get_component<PhysicsBody>(player);
            if (!map.is_entity_on_ground(player, coordinator)) {
                physics_system->update(update);
            }
            else {
                player_physics.velocity.y = 0;
            }
            std::cout << "Player y vel: " << player_physics.velocity.y << "\n";
            map.solve_collision(player, coordinator);
            renderer_system->set_camera_position(player_position);
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
            // std::cout << fps << "\n";
        }
    }
}
