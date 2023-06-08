#include "renderer.hpp"

#include <codecvt>

namespace etudegl {

SDL_WindowFlags Renderer::getWindowFlags() {
    SDL_WindowFlags window_flags =
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    return window_flags;
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

    window =
        SDL_CreateWindow("etudegl", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, width, height, window_flags);
    maincontext = SDL_GL_CreateContext(window);

    // SDL_GL_GetProcAddress get's OS specific OpenGL function
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return 1;
    }

    std::cout << "Renderer initialisation completed."
              << "\n";
    return 0;
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
                    framebuffer_size_callback(window, e.window.data1,
                                              e.window.data2);
                }
        }
    };

    return bQuit;
}

void Renderer::cleanup() { SDL_DestroyWindow(window); }

void Renderer::framebuffer_size_callback(SDL_Window *window, int width,
                                         int height) {
    //   glViewport(0, 0, width, height);
}

}  // namespace etudegl