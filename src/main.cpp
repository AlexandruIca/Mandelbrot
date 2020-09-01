///
/// Special thanks to: https://github.com/gendestry/Mandelbrot
///

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

    int width = default_width;
    int height = default_height;

    constexpr int default_itr = 200;
    int itr = default_itr;

    constexpr double default_zoom = 100.0;
    double zoom = default_zoom;

    constexpr double default_offset_x = 0.0;
    double offset_x = default_offset_x;

    constexpr double default_offset_y = 0.0;
    double offset_y = default_offset_y;

    double old_x = 0.0;
    double old_y = 0.0;

    assert(SDL_Init(SDL_INIT_VIDEO) == 0, "Couldn't initialize SDL");

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Mandelbrot",
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

    uniform int itr;
    uniform float zoom;
    uniform vec2 screen_size;
    uniform vec2 offset;

    float n = 0.0;
    float threshold = 100.0;

    float mandelbrot(vec2 c) {
        vec2 z = vec2(0.0, 0.0);

        for(int i = 0; i < itr; ++i) {
            vec2 znew;
            znew.x = (z.x * z.x) - (z.y * z.y) + c.x;
            znew.y = (2.0 * z.x * z.y) + c.y;
            z = znew;

            if((z.x * z.x) + (z.y * z.y) > threshold) {
                break;
            }

            n++;
        }

        return n / float(itr);
    }

    vec4 map_to_color(float t) {
        float r = 9.0 * (1.0 - t) * t * t * t;
        float g = 15.0 * (1.0 - t) * (1.0 - t) * t * t;
        float b = 8.5 * (1.0 - t) * (1.0 - t) * (1.0 - t) * t;

        return vec4(r, g, b, 1.0);
    }

    void main() {
        vec2 coord = gl_FragCoord.xy;
        float t = mandelbrot(((coord - screen_size / 2) / zoom) - offset);

        out_color = map_to_color(t);
    }
    )";

    auto const shader = create_program(desc);
    glUseProgram(shader);

    glUniform2f(glGetUniformLocation(shader, "screen_size"), default_width, default_height);
    glUniform2f(glGetUniformLocation(shader, "offset"), offset_x, offset_y);
    glUniform1f(glGetUniformLocation(shader, "zoom"), zoom);
    glUniform1i(glGetUniformLocation(shader, "itr"), itr);

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
            constexpr int itr_adder = 50;
            constexpr int itr_threshold = 100;

            switch(ev.type) {
            case SDL_WINDOWEVENT: {
                if(ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    glViewport(0, 0, ev.window.data1, ev.window.data2);
                    width = ev.window.data1;
                    height = ev.window.data2;
                    glUniform2f(glGetUniformLocation(shader, "screen_size"),
                                static_cast<float>(width),
                                static_cast<float>(height));
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                if(ev.button.button == SDL_BUTTON_LEFT) {
                    int x = 0;
                    int y = 0;
                    SDL_GetMouseState(&x, &y);

                    old_x = x;
                    old_y = y;
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
            case SDL_MOUSEWHEEL: {
                if(ev.wheel.y != 0) {
                    int x = 0;
                    int y = 0;
                    SDL_GetMouseState(&x, &y);

                    double dx = (x - width / 2) / zoom - offset_x;           // NOLINT
                    double dy = (height - y - height / 2) / zoom - offset_y; // NOLINT

                    offset_x = -dx;
                    offset_y = -dy;

                    constexpr double scroll_factor = 1.2;
                    if(ev.wheel.y < 0) {
                        zoom /= scroll_factor;
                    }
                    else {
                        zoom *= scroll_factor;
                    }

                    dx = (x - width / 2) / zoom;           // NOLINT
                    dy = (height - y - height / 2) / zoom; // NOLINT

                    offset_x += dx;
                    offset_y += dy;

                    glUniform1f(glGetUniformLocation(shader, "zoom"), zoom);
                    glUniform2f(glGetUniformLocation(shader, "offset"), offset_x, offset_y);
                }
                break;
            }
            case SDL_KEYDOWN: {
                if(ev.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
                else if(ev.key.keysym.sym == SDLK_c) {
                    // reset
                    itr = default_itr;
                    zoom = default_zoom;
                    offset_x = default_offset_x;
                    offset_y = default_offset_y;

                    glUniform1f(glGetUniformLocation(shader, "zoom"), zoom);
                    glUniform2f(glGetUniformLocation(shader, "offset"), offset_x, offset_y);
                }
                else if(ev.key.keysym.sym == SDLK_q) {
                    itr += itr_adder;
                }
                else if(ev.key.keysym.sym == SDLK_e) {
                    (itr > itr_threshold) ? itr -= itr_adder : itr = itr_adder;
                }

                glUniform1i(glGetUniformLocation(shader, "itr"), itr);

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

        if(dragging) {
            int mouse_x = 0;
            int mouse_y = 0;

            SDL_GetMouseState(&mouse_x, &mouse_y);

            offset_x += (mouse_x - old_x) / zoom;
            offset_y += (old_y - mouse_y) / zoom;

            old_x = mouse_x;
            old_y = mouse_y;

            glUniform2f(glGetUniformLocation(shader, "offset"), offset_x, offset_y);
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
