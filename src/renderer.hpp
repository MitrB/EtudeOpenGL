#pragma once

#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_video.h>

#include <iostream>

namespace etudegl {
class Renderer {
public:
    int width = 640;
    int height = 640;

    SDL_Window *window{nullptr};
    SDL_GLContext maincontext;

    int init();
    bool run();  // poll SDL input events
    void draw();
    void cleanup();

    // OpenGL specific flags
    SDL_WindowFlags getWindowFlags();
    void framebuffer_size_callback(SDL_Window *window, int width, int height);
};
}  // namespace etudegl