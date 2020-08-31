#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

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

[[nodiscard]] auto create_shader(std::string const& source, int const type) noexcept -> unsigned int
{
    unsigned int shader_id = glCreateShader(type);
    char const* src = source.c_str();
    glShaderSource(shader_id, 1, &src, nullptr);
    glCompileShader(shader_id);

    int succeded = 0;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &succeded);

    if(!static_cast<bool>(succeded)) {
        int len = 0;
        glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &len);

        std::string msg;
        msg.resize(static_cast<std::size_t>(len));

        glGetShaderInfoLog(shader_id, len, nullptr, msg.data());
        std::string shader_type = (type == GL_VERTEX_SHADER ? "vertex" : "fragment");
        assert(false, "Couldn't compile " + shader_type + " shader: " + msg);
    }

    return shader_id;
}

[[nodiscard]] auto create_program(program_description const& desc) noexcept -> unsigned int
{
    unsigned int program_id = glCreateProgram();

    auto const vs = create_shader(desc.vertex_shader_source, GL_VERTEX_SHADER);
    auto const fs = create_shader(desc.fragment_shader_source, GL_FRAGMENT_SHADER);

    glAttachShader(program_id, vs);
    glAttachShader(program_id, fs);

    glLinkProgram(program_id);

    int succeded = 0;
    glGetProgramiv(program_id, GL_LINK_STATUS, &succeded);

    if(!static_cast<bool>(succeded)) {
        int len = 0;
        glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &len);

        std::string msg;
        msg.resize(static_cast<std::size_t>(len));

        glGetProgramInfoLog(program_id, len, nullptr, msg.data());
        assert(false, "Coulnd't link program: " + msg);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program_id;
}

auto main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) noexcept -> int
{
    constexpr int default_width = 1280;
    constexpr int default_height = 720;

    /*
    constexpr int default_itr = 200;
    int itr = default_itr;

    constexpr double default_zoom = 100.0;
    double zoom = default_zoom;

    constexpr double default_offset_x = 0.0;
    double offset_x = default_offset_x;

    constexpr double default_offset_y = 0.0;
    double offset_y = default_offset_y;
    */

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

    std::vector<float> const vertices = {
        -1.0F, 1.0F,  // TOP LEFT
        -1.0F, -1.0F, // BOTTOM LEFT
        1.0F,  -1.0F, // BOTTOM RIGHT
        -1.0F, 1.0F,  // TOP LEFT
        1.0F,  1.0F,  // TOP RIGHT
        1.0F,  -1.0F  // BOTTOM_RIGHT
    };

    program_description desc;

    desc.vertex_shader_source = R"(
    #version 330 core

    layout(location = 0) in vec2 pos;

    void main() {
        gl_Position = vec4(pos, 0.0, 1.0);
    }
    )";

    desc.fragment_shader_source = R"(
    #version 330 core

    out vec4 out_color;

    void main() {
        out_color = vec4(1.0, 0.5, 0.7, 1.0);
    }
    )";

    auto const shader = create_program(desc);
    glUseProgram(shader);

    unsigned int vao = 0;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    unsigned int vbo = 0;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glClearColor(0.0F, 0.0F, 0.0F, 1.0F);

    bool running = true;
    bool dragging = false;

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
            case SDL_MOUSEBUTTONDOWN: {
                if(ev.button.button == SDL_BUTTON_LEFT) {
                    dragging = true;
                }
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                if(ev.button.button == SDL_BUTTON_LEFT) {
                    dragging = false;
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

        int mouse_x = 0;
        int mouse_y = 0;

        if(dragging) {
            SDL_GetMouseState(&mouse_x, &mouse_y);
            std::cout << "Dragging mouse at (" << mouse_x << ", " << mouse_y << ')' << std::endl;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_TRIANGLES, 0, vertices.size());

        SDL_GL_SwapWindow(window);
    }

    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteProgram(shader);

    SDL_GL_DeleteContext(ctx);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
