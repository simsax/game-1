#include <fstream>
#include <iostream>
#include <sstream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include "Config.h"

#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define SOURCE_DIR std::string(PROJECT_SOURCE_DIR)

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

std::string readFile(const std::string& path) {
    std::ifstream inputFile{ path };
    std::stringstream fileString;

    if (!inputFile) {
        std::cerr << "Failed to open file: " << path << "\n";
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(inputFile, line)) {
        fileString << line << "\n";
    }

    return fileString.str();
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
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
    glEnableVertexAttribArray(0);
    
    // compile vertex shader
    std::string vertexShaderSource = readFile(SOURCE_DIR + "/res/shaders/shader.vert");
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vs = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    int success;
    char info[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, info);
        fprintf(stderr, "Vertex shader compilation failed with error: %s\n", info);
        exit(EXIT_FAILURE);
    }

    // compile fragment shader
    std::string fragmentShaderSource = readFile(SOURCE_DIR + "/res/shaders/shader.frag");
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fs = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, info);
        fprintf(stderr, "Fragment shader compilation failed with error: %s\n", info);
        exit(EXIT_FAILURE);
    }

    // link shaders to shader program
    uint32_t shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, info);
        fprintf(stderr, "Shader linking failed with error: %s\n", info);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glUseProgram(shaderProgram);

    int exit = 0;
    while(!exit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    exit = 1;
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        exit = 1;
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
            
            glUseProgram(shaderProgram);
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
