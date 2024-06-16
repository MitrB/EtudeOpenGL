#include "renderer.hpp"
#include <GLFW/glfw3.h>
#include <glm/ext/matrix_transform.hpp>
#include "ecs.h"
#include "structs.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "../third_party/stb_image/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../third_party/tiny_obj_loader/tiny_obj_loader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }

void Renderer::mouse_callback_static(GLFWwindow* window, double xpos, double ypos) {
    Renderer* renderer = static_cast<Renderer*>(glfwGetWindowUserPointer(window));
    if (renderer) {
        renderer->mouse_callback(window, xpos, ypos);
    }
}

void Renderer::mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);
    if (debug_mode) {
        mouse_offset = glm::vec2{xpos, ypos};
        return;
    }
    float delta_x = xpos - mouse_last_position.x;
    float delta_y = ypos - mouse_last_position.y;
    mouse_last_position = glm::vec2{xpos, ypos};

    const float sensitivity = 0.1f;
    delta_x *= sensitivity;
    delta_y *= sensitivity;

    yaw += delta_x;
    pitch -= delta_y;

    if (pitch < -89.0f) {
        pitch = -89.0f;
    }
    if (pitch > 89.0f) {
        pitch = 89.0f;
    }

    glm::vec3 direction{};
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(direction);
};

void Renderer::init() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "EtudeGL", NULL, NULL);
    glfwSetWindowUserPointer(window, this);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

}

Model Renderer::load_model(const char* path) {

    std::string model_file = path;
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "../assets";

    tinyobj::ObjReader reader;

    reader.ParseFromFile(model_file);
    if (!reader.ParseFromFile(model_file, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader: " << reader.Error();
        }
        exit(1);
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();
    auto& materials = reader.GetMaterials();

    Model model;

    std::unordered_map<Vertex, uint32_t> unique_vertices;

    // TODO: load textures and materials
    // Loop over shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            Vertex vertex{};

            if (index.vertex_index >= 0) {
                vertex.position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2],
                };
            }

            if (index.normal_index >= 0) {
                vertex.normal = {
                    attrib.normals[3 * index.normal_index + 0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2],
                };
            }

            if (unique_vertices.count(vertex) == 0) {
                unique_vertices[vertex] = static_cast<uint32_t>(model.vertices.size());
                model.vertices.push_back(vertex);
            }
            model.indices.push_back(unique_vertices[vertex]);
        }
    }

    return model;
}



void Renderer::update(Update update) {
    Model model = load_model("../assets/cube.obj");
    Shader shader{"../shaders/basic_shader.vert", "../shaders/basic_shader.frag"};
    Shader light_shader{"../shaders/light_shader.vert", "../shaders/light_shader.frag"};

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, model.vertices.size() * sizeof(Vertex), model.vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(uint16_t), model.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.indices.size() * sizeof(uint16_t), model.indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);

    // space
    glm::mat4 model_space = glm::mat4(1.0f);

    view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(85.0f), (float)WIDTH/(float)HEIGHT, 0.1f, 100.0f);
            
    glfwSetCursorPosCallback(window, mouse_callback_static);

    interaction_timeout -= update.delta;
    if (interaction_timeout < 0.0) {
        interaction_timeout = 0.0;
    }

    glfwPollEvents();

    // Input
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS and interaction_timeout <= 0) {
        if (debug_mode) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            mouse_last_position = mouse_offset;
            debug_mode = false;
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            debug_mode = true;
        }
        interaction_timeout = interaction_timeout_max;
    }

    if (!debug_mode) {
        const float cameraSpeed = 5.0f * update.delta;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_position += cameraSpeed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_position -= cameraSpeed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
    }


    // sync

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // rendering
    shader.use();
    shader.setVec3("object_color", 1.0f, 0.5f, 0.31f);
    shader.setVec3("light_color", 1.0f, 1.0f, 1.0f);
    shader.setVec3("light_pos", light_pos.x, light_pos.y, light_pos.z);
    shader.setVec3("view_pos", camera_position);
    shader.setVec3("material.ambient", ambient);
    shader.setVec3("material.diffuse", diffuse);
    shader.setVec3("material.specular", specular);
    shader.setFloat("material.shininess", shininess);
    shader.setVec3("light.ambient", l_ambient);
    shader.setVec3("light.diffuse", l_diffuse);
    shader.setVec3("light.specular", l_specular);
    shader.setVec3("light.position", light_pos);

    for (Entity entity : entities) {
        model_space = glm::mat4(1.0f);
        model_space = glm::translate(model_space, update.coordinator->get_component<PhysicsBody>(entity).position);
        view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

        unsigned int model_location = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model_space));

        unsigned int view_location = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));

        unsigned int projection_location = glGetUniformLocation(shader.ID, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_SHORT, 0);
    }


    // shader.use();
    // model_space = glm::mat4(1.0f);
    // model_space = glm::translate(model_space, glm::vec3(1.0f, 0, 0));
    // shader.setMat4("model", model_space);
    // glBindVertexArray(VAO);
    // glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_SHORT, 0);

    // light_shader.use();
    // light_shader.setMat4("projection", projection);
    // light_shader.setMat4("view", view);
    // model_space = glm::mat4(1.0f);
    // model_space = glm::translate(model_space, light_pos);
    // model_space = glm::scale(model_space, glm::vec3(0.2f));
    // light_shader.setMat4("model", model_space);

    // glBindVertexArray(lightVAO);
    // glDrawElements(GL_TRIANGLES, model.indices.size(), GL_UNSIGNED_SHORT, 0);

    // o

    if (debug_mode) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ColorEdit3("Ambient Color", &ambient.r);
        ImGui::ColorEdit3("Diffuse Color", &diffuse.r);
        ImGui::ColorEdit3("Specular Color", &specular.r);
        ImGui::InputFloat("Shininess", &shininess);

        ImGui::ColorEdit3("Light Ambient Color", &l_ambient.r);
        ImGui::ColorEdit3("Light Diffuse Color", &l_diffuse.r);
        ImGui::ColorEdit3("Light Specular Color", &l_specular.r);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    // check and call events and swap buffers
    glfwSwapBuffers(window);

}

bool Renderer::close_window() {
    return glfwWindowShouldClose(window);
}


void Renderer::cleanup() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

Renderer::~Renderer() {
    cleanup();
}