#include "renderer.hpp"

#include <SDL2/SDL_opengl.h>

#include <codecvt>
#include <vector>

namespace etudegl {

SDL_WindowFlags Renderer::getWindowFlags() {
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    return window_flags;
}

void Renderer::draw() {
    glClearColor(0.8f, 0.9f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw our first triangle
    basicShader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, trianglesToDrawExpressedInVertices, GL_UNSIGNED_INT, 0);
    // glBindVertexArray(0); // no need to unbind it every time

    SDL_GL_SwapWindow(window);
}

int Renderer::init() {
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    SDL_Init(SDL_INIT_VIDEO);

    SDL_WindowFlags window_flags = Renderer::getWindowFlags();

    window = SDL_CreateWindow("etudegl", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
    maincontext = SDL_GL_CreateContext(window);

    // SDL_GL_GetProcAddress get's OS specific OpenGL function
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    glViewport(0, 0, width, height);

    basicShader.create("../shaders/basic_shader.vert", "../shaders/basic_shader.frag");

    std::cout << "Renderer initialisation completed."
              << "\n";
    return 0;
}

void Renderer::createBuffers(std::vector<float> &vertices, std::vector<uint> &indices) {
    trianglesToDrawExpressedInVertices = indices.size();
    for (auto &vertex : vertices) {
        vertex = vertex / 26.0f;
        std::cout << vertex << " ";
    }
    std::cout << "\n";

    for (auto &index : indices) {
        std::cout << index << " ";
    }
    std::cout << "\n";

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(vertices[0]), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound
    // vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO;
    // keep the EBO bound.
    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens.
    // Modifying other VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs)
    // when it's not directly necessary.
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

bool Renderer::run() {
    SDL_Event e;
    bool bQuit = false;

    // Handle events on queue
    while (SDL_PollEvent(&e) != 0) {
        // close the window when user alt-f4s or clicks the X button
        switch (e.type) {
            case SDL_QUIT:
                std::cout << "QUIT CALL"
                          << "\n";
                bQuit = true;
            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    framebuffer_size_callback(window, e.window.data1, e.window.data2);
                }
        }
    };
    draw();

    return bQuit;
}

void Renderer::cleanup() { SDL_DestroyWindow(window); }

void Renderer::framebuffer_size_callback(SDL_Window *window, int width, int height) { glViewport(0, 0, width, height); }

}  // namespace etudegl