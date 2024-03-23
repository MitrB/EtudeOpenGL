#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <glm/detail/qualifier.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <unordered_map>
#include <vector>

#include "../third_party/fmt/include/fmt/core.h"
#include "../third_party/fmt/include/fmt/format.h"
#include "shader.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "../third_party/stb_image/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../third_party/imgui/imgui_impl_glfw.h"
#include "../third_party/imgui/imgui_impl_opengl3.h"
#include "../third_party/tiny_obj_loader/tiny_obj_loader.h"

bool debug_mode = true;
glm::vec2 mouse_offset{};

float interaction_timeout_max = 0.1;
float interaction_timeout = 0;

// lighting
glm::vec3 light_pos(2.0f, 2.0f, 2.0f);

// light values
// glm::vec3 l_ambient{0.2, 0.2, 0.2};
// glm::vec3 l_diffuse{0.5, 0.5, 0.5};
// glm::vec3 l_specular{1.0, 1.0, 1.0};
glm::vec3 l_ambient{1.0, 1.0, 1.0};
glm::vec3 l_diffuse{1.0, 1.0, 1.0};
glm::vec3 l_specular{1.0, 1.0, 1.0};

glm::vec3 ambient{0.0, 0.05, 0.05};
glm::vec3 diffuse{0.4, 0.5, 0.5};
glm::vec3 specular{0.04, 0.7, 0.7};

float shininess = 32.0f;

struct Vertex {
    glm::vec3 position{};
    glm::vec3 normal{};

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

static constexpr int WIDTH = 1920 / 2;
static constexpr int HEIGHT = 1080 / 2;

glm::vec2 mouse_last_position{WIDTH / 2, HEIGHT / 2};
glm::vec3 camera_front{0.0f, 0.0f, -1.0f};
glm::vec3 camera_position{0.0f, 0.0f, 3.0f};
glm::vec3 camera_up{0.0f, 1.0f, 0.0f};
glm::mat4 view{};

float yaw{-90.0f};
float pitch{0.0f};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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

int main() {
    fmt::print("Hello OpenGl\n");

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "EtudeGL", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glViewport(0, 0, WIDTH, HEIGHT);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // imgui

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;   // Enable Gamepad Controls

    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // tiny_obj

    std::string mido_model_file = "../assets/cube.obj";
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = "../assets";

    tinyobj::ObjReader reader;

    reader.ParseFromFile(mido_model_file);
    if (!reader.ParseFromFile(mido_model_file, reader_config)) {
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

    std::vector<Vertex> vertices{};
    std::vector<uint16_t> indices{};

    std::unordered_map<Vertex, uint32_t> unique_vertices;

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
                unique_vertices[vertex] = static_cast<uint32_t>(vertices.size());
                vertices.push_back(vertex);
            }
            indices.push_back(unique_vertices[vertex]);
        }
    }

    // read shaders
    Shader shader{"../shaders/basic_shader.vert", "../shaders/basic_shader.frag"};
    Shader light_shader{"../shaders/light_shader.vert", "../shaders/light_shader.frag"};

    // buffers
    unsigned int VBO, EBO, VAO, lightVAO;

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);

    // wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);

    // space
    glm::mat4 model = glm::mat4(1.0f);

    view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
            
    glfwSetCursorPosCallback(window, mouse_callback);

    float delta = 0.0f;
    float last_frame = 0.0f;
    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    while (!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;

        interaction_timeout -= delta;
        if (interaction_timeout < 0.0) {
            interaction_timeout = 0.0;
        }

        // light post calc
        float angle_a = 0.5 * delta;
        light_pos = glm::vec3(light_pos.x * cos(angle_a) - light_pos.z * sin(angle_a), light_pos.y,
                              light_pos.x * sin(angle_a) + light_pos.z * cos(angle_a));

        glfwPollEvents();

        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS and interaction_timeout <= 0) {
            // glfwSetWindowShouldClose(window, true);
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

        // if (!debug_mode) {
            const float cameraSpeed = 5.0f * delta;  // adjust accordingly
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_position += cameraSpeed * camera_front;
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_position -= cameraSpeed * camera_front;
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
        // }


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

        model = glm::mat4(1.0f);
        view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

        unsigned int model_location = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

        unsigned int view_location = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));

        unsigned int projection_location = glGetUniformLocation(shader.ID, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

        light_shader.use();
        light_shader.setMat4("projection", projection);
        light_shader.setMat4("view", view);
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pos);
        model = glm::scale(model, glm::vec3(0.2f));
        light_shader.setMat4("model", model);

        glBindVertexArray(lightVAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

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

    glDeleteVertexArrays(1, &VAO);
    glDeleteVertexArrays(1, &lightVAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}