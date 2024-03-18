#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>
#include <pthread.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <glm/detail/qualifier.hpp>
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
#include "../third_party/tiny_obj_loader/tiny_obj_loader.h"

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

    // models

    // tiny_obj

    std::string mido_model_file = "../assets/mido_01.obj";
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

    // textures
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char* data = stbi_load("../assets/container.jpg", &width, &height, &nrChannels, 0);

    unsigned int texture0;
    glGenTextures(1, &texture0);
    // texture params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glBindTexture(GL_TEXTURE_2D, texture0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    data = stbi_load("../assets/face.png", &width, &height, &nrChannels, 0);
    unsigned int texture1;
    glGenTextures(1, &texture1);
    // texture params
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, texture1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);

    // buffers
    unsigned int VBO, EBO, VAO;

    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    // glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint16_t), indices.data(), GL_STATIC_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));  // texuture
    // glEnableVertexAttribArray(2);

    // wireframe mode
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    glEnable(GL_DEPTH_TEST);

    // space
    glm::mat4 model = glm::mat4(1.0f);

    view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

    glm::mat4 projection;
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // prepare draw call
    shader.use();
    // shader.setFloat("mix_value", 0.1);
    // shader.setInt("texture0", 0);
    // shader.setInt("texture1", 1);

    float delta = 0.0f;
    float last_frame = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float current_frame = glfwGetTime();
        delta = current_frame - last_frame;
        last_frame = current_frame;

        // Input
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        const float cameraSpeed = 5.0f * delta;  // adjust accordingly
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) camera_position += cameraSpeed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) camera_position -= cameraSpeed * camera_front;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera_position -= glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera_position += glm::normalize(glm::cross(camera_front, camera_up)) * cameraSpeed;

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetCursorPosCallback(window, mouse_callback);

        // sync

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::rotate(model, (float)glfwGetTime() / 800.0f, glm::vec3(0.0f, 1.0f, 0.0f));

        view = glm::lookAt(camera_position, camera_position + camera_front, camera_up);

        unsigned int model_location = glGetUniformLocation(shader.ID, "model");
        glUniformMatrix4fv(model_location, 1, GL_FALSE, glm::value_ptr(model));

        unsigned int view_location = glGetUniformLocation(shader.ID, "view");
        glUniformMatrix4fv(view_location, 1, GL_FALSE, glm::value_ptr(view));

        unsigned int projection_location = glGetUniformLocation(shader.ID, "projection");
        glUniformMatrix4fv(projection_location, 1, GL_FALSE, glm::value_ptr(projection));

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // rendering
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, texture0);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, texture1);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_SHORT, 0);

        // check and call events and swap buffers
        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    glfwTerminate();
    return 0;
}