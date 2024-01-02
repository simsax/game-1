#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include "Config.h"
#include "utils.h"
#include "shader.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define ABS_PATH(x) (std::string(PROJECT_SOURCE_DIR) + x).c_str()

#define SDL_ERROR() fprintf(stderr, "SDL_Error: %s\n", SDL_GetError())
#define BLACK 0,0,0

template <typename T>
T* SDL(T* ptr) {
    if (ptr == NULL) {
        SDL_ERROR();
        exit(EXIT_FAILURE);
    }
    return ptr;
}

int main(void) {
    // no error checking
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_ERROR();
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window *window = SDL<SDL_Window>(SDL_CreateWindow(
        "Game 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    ));

    SDL_GLContext context = SDL_GL_CreateContext(window);

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

    // opengl stuff

    float vertices[] = {
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, // bottom left
        -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, 1.0f   // top left 
    };

    uint32_t indices[] = {  // note that we start from 0!
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };  

    // create vao
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create vbo
    uint32_t vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // create ebo (if not using triangle_strip)
    uint32_t ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // specify vertex attributes
    float stride = 6 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // compile and link shaders
    Shader shader = Shader(
            ABS_PATH("/res/shaders/shader.vert"),
            ABS_PATH("/res/shaders/shader.frag"));
    shader.bind();

    int quit = 0;
    while(!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = 1;
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        quit = 1;
                    }
                    break;
                default:
                    break;
            }
        }


        // render
        {
            glClearColor(BLACK, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            float time = SDL_GetTicks() / 1000.0f;
            float greenValue = sin(time) * 0.5 + 0.5;

            shader.bind();
            shader.setUniform4f("uColor", 0, greenValue, 0, 1);

            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            /* glDrawArrays(GL_TRIANGLES, 0, 3); */
        }

        SDL_GL_SwapWindow(window);
    }


    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
