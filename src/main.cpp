#include <cstdlib>
#include <fstream>
#include <iostream>
#include <glad/glad.h>
#include <SDL2/SDL.h>
#include <array>
#include "Config.h"
#include "utils.h"
#include "shader.h"
// move this to texture class
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// this will be changed dynamically to allow resizing
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

#define ABS_PATH(x) (std::string(PROJECT_SOURCE_DIR) + (x))

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

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture;
};

int main(void) {
    // no error checking
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_ERROR();
        exit(EXIT_FAILURE);
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
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

    /* std::array<Vertex, 4> vertices = {{ */
    /*     {glm::vec3(0.5f,  0.5f, 0.0f),    glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},  // top right */
    /*     {glm::vec3(0.5f, -0.5f, 0.0f),    glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},  // bottom right */
    /*     {glm::vec3(-0.5f, -0.5f, 0.0f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},  // bottom left */
    /*     {glm::vec3(-0.5f,  0.5f, 0.0f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)}   // top left */ 
    /* }}; */

    std::array<Vertex, 24> vertices = {{
        // front
        {glm::vec3(0.5f,   0.5f, 0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  -0.5f, 0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, 0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},

        // back
        {glm::vec3(0.5f,   0.5f, -0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  -0.5f, -0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},

        // down
        {glm::vec3(0.5f,  -0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  -0.5f, -0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},

        // up
        {glm::vec3(0.5f,  0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},

        // left
        {glm::vec3(-0.5f,  0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},

        // right
        {glm::vec3(0.5f,  0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
    }};

    std::array<uint32_t, 36> indices;
    int ix = 0;
    for (int i = 0; i < 24; i += 4) {
        indices[ix++] = i + 0;
        indices[ix++] = i + 1;
        indices[ix++] = i + 3;
        indices[ix++] = i + 1;
        indices[ix++] = i + 2;
        indices[ix++] = i + 3;
    }

    // create vao
    uint32_t vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // create vbo
    uint32_t vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);

    // create ebo (if not using triangle_strip)
    uint32_t ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices.data(), GL_STATIC_DRAW);

    // specify vertex attributes
    float stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*) 0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*) (3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*) (6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // compile and link shaders
    Shader shader = Shader(
            ABS_PATH("/res/shaders/shader.vert").c_str(),
            ABS_PATH("/res/shaders/shader.frag").c_str());
    shader.bind();

    // texture 1
    uint32_t texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load PNG image
    int width, height, nrChannels;
    std::string imagePath = ABS_PATH("/res/textures/container.jpg");
	// flip texture upside down because for opengl bottom left is the starting position
	stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0); 
    if (data == nullptr) {
        fprintf(stderr, "Failed at loading image: %s\n", imagePath.c_str());
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    // texture 2
    uint32_t texture2;
    glGenTextures(1, &texture2);
    glBindTexture(GL_TEXTURE_2D, texture2);
    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load second image
    imagePath = ABS_PATH("/res/textures/awesomeface.png");
    data = stbi_load(imagePath.c_str(), &width, &height, &nrChannels, 0);
    stbi_set_flip_vertically_on_load(true);
    if (data == nullptr) {
        fprintf(stderr, "Failed at loading image: %s\n", imagePath.c_str());
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    shader.bind();
    shader.setUniform1i("texture1", 0);
    shader.setUniform1i("texture2", 1);

    // this was in the render loop
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture2);

    glEnable(GL_DEPTH_TEST);

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
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float time = SDL_GetTicks() / 1000.0f;
            /* float greenValue = sin(time) * 0.5 + 0.5; */

            shader.bind();
            /* shader.setUniform4f("uColor", 0, greenValue, 0, 1); */

            // weird rotating plane

            // order of transformation is reversed (the last one is the first which is applied)
            glm::mat4 model = glm::mat4(1.0f);
            /* model = glm::translate(model, glm::vec3(0.5f, -0.5f, 0.0f)); */
            model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
            model = glm::rotate(model, time, glm::vec3(0.0f, 0.0f, -1.0f));

            glm::mat4 view = glm::mat4(1.0f);
            // note that we're translating the scene in the reverse direction of where we want to move
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f)); 
            
            glm::mat4 projection = glm::perspective(glm::radians(70.0f), ASPECT_RATIO, 0.1f, 100.0f);

            // TODO: figure out if better to compute mvp matrix on cpu vs doing it in vertex shader
            shader.SetUniformMatrix4fv("model", model);
            shader.SetUniformMatrix4fv("view", view);
            shader.SetUniformMatrix4fv("projection", projection);

            glBindVertexArray(vao);
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
            /* glDrawArrays(GL_TRIANGLES, 0, 3); */
        }

        SDL_GL_SwapWindow(window);
    }


    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
