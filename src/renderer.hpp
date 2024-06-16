#pragma once

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
#include "ecs.h"
#include "shader.hpp"
#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include "../third_party/stb_image/stb_image.h"
#include "../third_party/imgui/imgui_impl_glfw.h"
#include "../third_party/imgui/imgui_impl_opengl3.h"
#include "../third_party/tiny_obj_loader/tiny_obj_loader.h"

#include "structs.hpp"



class Renderer : public System {
    public:
        Renderer();
        ~Renderer();
        bool close_window();
        Model load_model(const char* path);

        void init();
        void update(Update update);
        void cleanup();

    private:
        // draw

        unsigned int VBO, EBO, VAO, lightVAO;
        Model model;
        Shader* shader;

        static void mouse_callback_static(GLFWwindow* window, double xpos, double ypos);
        void mouse_callback(GLFWwindow* window, double xpos, double ypos);
        
        GLFWwindow* window;
        static constexpr int WIDTH = 1920;
        static constexpr int HEIGHT = 1080;

        bool debug_mode = true;
        glm::vec2 mouse_offset{};

        float interaction_timeout_max = 0.1;
        float interaction_timeout = 0;

        glm::vec2 mouse_last_position{WIDTH / 2, HEIGHT / 2};
        glm::vec3 camera_front{0.0f, 0.0f, -1.0f};
        glm::vec3 camera_position{0.0f, 0.0f, 3.0f};
        glm::vec3 camera_up{0.0f, 1.0f, 0.0f};
        glm::mat4 view{};

        float yaw{-90.0f};
        float pitch{0.0f};

        // lighting
        glm::vec3 light_pos{2.0f, 2.0f, 2.0f};

        // light values
        // glm::vec3 l_ambient{0.2, 0.2, 0.2};
        // glm::vec3 l_diffuse{0.5, 0.5, 0.5};
        // glm::vec3 l_specular{1.0, 1.0, 1.0};
        float shininess = 32.0f;
        glm::vec3 l_ambient{1.0, 1.0, 1.0};
        glm::vec3 l_diffuse{1.0, 1.0, 1.0};
        glm::vec3 l_specular{1.0, 1.0, 1.0};

        glm::vec3 ambient{0.0, 0.05, 0.05};
        glm::vec3 diffuse{0.4, 0.5, 0.5};
        glm::vec3 specular{0.04, 0.7, 0.7};
};