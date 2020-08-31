#include <cstdlib>
#include <iostream>
#include <string>

#include <SDL.h>
#include <glad/glad.h>

auto assert(bool const cond, std::string const& msg = "") noexcept -> void
{
    if(!cond) {
        std::cout << "Assertion failed! " << msg << std::endl;
        std::exit(EXIT_FAILURE);
    }
}

struct program_description
{
    std::string vertex_shader_source;
    std::string fragment_shader_source;
};

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept -> int
{
    constexpr int default_width = 1280;
    constexpr int default_height = 720;

    assert(SDL_Init(SDL_INIT_VIDEO) == 0, "Couldn't initialize SDL");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Mandlebrot",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          default_width,
                                          default_height,
                                          SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    assert(window != nullptr, "Couldn't create window!");

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    SDL_GLContext ctx = SDL_GL_CreateContext(window);
    assert(gladLoadGLLoader(static_cast<GLADloadproc>(SDL_GL_GetProcAddress)) != 0, "Couldn't create OpenGL context!");

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    bool running = true;

    while(running) {
        SDL_Event ev;
        while(static_cast<bool>(SDL_PollEvent(&ev))) {
            switch(ev.type) {
            case SDL_WINDOWEVENT: {
                if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    glViewport(0, 0, ev.window.data1, ev.window.data2);
                }
                break;
            }
            case SDL_KEYDOWN: {
                if(ev.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                break;
            }
            case SDL_QUIT: {
                running = false;
                break;
            }
            default: {
                break;
            }
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
