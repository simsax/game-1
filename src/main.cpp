#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_stdinc.h>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include "render.h"
#include <SDL2/SDL.h>
#include <array>
#include "Config.h"
#include "utils.h"
#include "shader.h"
#include "vao.h"
#include "texture.h"

#ifndef NDEBUG
#define DEBUG
#endif

// this will be changed dynamically to allow resizing
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080
#define ASPECT_RATIO ((float)SCREEN_WIDTH / (float)SCREEN_HEIGHT)

#define ABS_PATH(x) (PROJECT_SOURCE_DIR x)

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

int tiles[5][5] = {
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
    { 1, 1, 1, 1, 1 },
};


int main() {
    // no error checking
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_ERROR();
        exit(EXIT_FAILURE);
    }

    // I am only going to use opengl 4.1 features, besides the debug output (available from 4.3)
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#ifdef DEBUG
    printf("DEBUG ON\n");
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    SDL_Window *window = SDL<SDL_Window>(SDL_CreateWindow(
        "Game 1",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
    ));

    // hide mouse and keep it inside the window
    SDL_SetRelativeMouseMode(SDL_TRUE);

    SDL_GLContext context = SDL(SDL_GL_CreateContext(window));

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

    // opengl loaded
#ifdef DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        // initialize debug output 
        printf("Debug output is on\n");
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        // errors only
        /* glDebugMessageControl(GL_DEBUG_SOURCE_API, */ 
        /*                       GL_DEBUG_TYPE_ERROR, */ 
        /*                       GL_DEBUG_SEVERITY_HIGH, */
        /*                       0, nullptr, GL_TRUE); */ 
    }
#endif


    // each quad will have a margin (I think I can do this in the fragment shader).
    // the cube will have margins too

    // need to abstract rendering of basic shapes like a quad at coord x,y,z
    static constexpr int numTilesVertices = 25 * 4;
    std::vector<Vertex> tilesVertices;
    tilesVertices.reserve(numTilesVertices);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            tilesVertices.emplace_back(glm::vec3(j,   0.0f, i),   glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec2(0.0f, 0.0f));
            tilesVertices.emplace_back(glm::vec3(j + 1,   0.0f, i),   glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec2(1.0f, 0.0f));
            tilesVertices.emplace_back(glm::vec3(j + 1,  0.0f, i + 1),   glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec2(1.0f, 1.0f));
            tilesVertices.emplace_back(glm::vec3(j,  0.0f, i + 1),   glm::vec3(1.0f, 1.0f, 1.0f),  glm::vec2(0.0f, 1.0f));
        }
    }

    std::vector<Vertex> cubeVertices = {
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
        {glm::vec3(-0.5f,  -0.5f, 0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},

        // right
        {glm::vec3(0.5f,  0.5f,  0.5f),   glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f),   glm::vec3(1.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f),   glm::vec3(0.0f, 0.0f, 1.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f),   glm::vec3(0.0f, 1.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
    };

    std::vector<uint32_t> cubeIndices = generateQuadIndices(6);
    std::vector<uint32_t> tilesIndices = generateQuadIndices(25);

    std::vector<Layout> cubeLayout = {
        { GL_FLOAT, 3 },
        { GL_FLOAT, 3 },
        { GL_FLOAT, 2 }
    };

    Vao cubeVao = Vao(cubeVertices, cubeIndices, cubeLayout, GL_STATIC_DRAW);
    
    // compile and link shaders
    Shader shader = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/cubeShader.frag"));
    shader.Bind();

    Texture texture1 = Texture(ABS_PATH("/res/textures/container.jpg"), GL_RGB, GL_RGB);
    Texture texture2 = Texture(ABS_PATH("/res/textures/awesomeface.png"), GL_RGB, GL_RGBA);

    shader.SetUniform1i("texture1", 0);
    shader.SetUniform1i("texture2", 1);

    std::vector<Layout> tilesLayout = {
        { GL_FLOAT, 3 },
        { GL_FLOAT, 3 },
        { GL_FLOAT, 2 }
    };

    Vao tilesVao = Vao(tilesVertices, tilesIndices, tilesLayout, GL_STATIC_DRAW);

    // compile and link shaders
    Shader shader2 = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/tileShader.frag"));
    shader2.Bind();

    glEnable(GL_DEPTH_TEST);

    // some globals (outside the game loop)
    bool quit = false;

    bool forward = false;
    bool backwards = false;
    bool left = false;
    bool right = false;
    bool up = false;
    bool down = false;

    float deltaTime = 0.0f; // time between current and last frame
    float lastFrame = 0.0f; // time of last frame

    // camera
    glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);
    static constexpr float cameraSpeed = 5.0f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    int xoffset = 0;
    int yoffset = 0;
    static constexpr float mouseSensitivity = 0.1f;
    static constexpr float defaultFov = 70.0f;
    float fov = defaultFov;

    // handle input
    while(!quit) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        quit = true;
                        break;
                    case SDLK_w:
                        forward = true;
                        break;
                    case SDLK_a:
                        left = true;
                        break;
                    case SDLK_s:
                        backwards = true;
                        break;
                    case SDLK_d:
                        right = true;
                        break;
                    case SDLK_SPACE:
                        up = true;
                        break;
                    case SDLK_LCTRL:
                        down = true;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                    case SDLK_w:
                        forward = false;
                        break;
                    case SDLK_a:
                        left = false;
                        break;
                    case SDLK_s:
                        backwards = false;
                        break;
                    case SDLK_d:
                        right = false;
                        break;
                    case SDLK_SPACE:
                        up = false;
                        break;
                    case SDLK_LCTRL:
                        down = false;
                        break;
                    default:
                        break;
                    }
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    /* if (event.button.button == SDL_BUTTON_LEFT) { */
                    /*     mouse_down(LEFT_BUTTON); */
                    /*     drag(); */
                    /* } else if (event.button.button == SDL_BUTTON_RIGHT) { */
                    /*     mouse_down(RIGHT_BUTTON); */
                    /* } */
                    break;
                case SDL_MOUSEBUTTONUP:
                    /* if (event.button.button == SDL_BUTTON_LEFT) { */
                    /*     mouse_up(LEFT_BUTTON); */
                    /*     undrag(); */
                    /* } else if (event.button.button == SDL_BUTTON_RIGHT) { */
                    /*     mouse_up(RIGHT_BUTTON); */
                    /* } */
                    break;
                case SDL_MOUSEMOTION:
                    {
                        // retrieve x and y offset from mouse movement
                        xoffset = event.motion.xrel;
                        yoffset = - event.motion.yrel; // down is 1 for SDL but -1 for OpenGL coords
                        yaw = glm::mod(yaw + xoffset * mouseSensitivity, 360.0f);
                        pitch += yoffset * mouseSensitivity;

                        // constrain pitch to avoid weird camera movements
                        if (pitch > 89.0f)
                            pitch = 89.0f;
                        if (pitch < -89.0f)
                            pitch = -89.0f;

                        // calculate camera direction
                        glm::vec3 direction;
                        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
                        direction.y = sin(glm::radians(pitch));
                        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
                        cameraFront = glm::normalize(direction);
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    {
                        fov -= event.wheel.y;

                        // constain fov
                        if (fov < 1.0f)
                            fov = 1.0f;
                        if (fov > defaultFov)
                            fov = defaultFov;
                    }
                default:
                    break;
            }
        }

        // update
        float time = SDL_GetTicks() / 1000.0f;
        deltaTime = time - lastFrame;
        lastFrame = time;
        float cameraOffset = cameraSpeed * deltaTime;
        glm::vec3 yMasked = glm::vec3(1.0f, 0.0f, 1.0f);

        if (forward)
            cameraPos += yMasked * cameraOffset * cameraFront;
        if (backwards)
            cameraPos -= yMasked * cameraOffset * cameraFront;
        if (left)
            cameraPos -= yMasked * cameraOffset * glm::normalize(glm::cross(cameraFront, cameraUp));
        if (right)
            cameraPos += yMasked * cameraOffset * glm::normalize(glm::cross(cameraFront, cameraUp));
        if (up)
            cameraPos.y += cameraOffset;
        if (down)
            cameraPos.y -= cameraOffset;

        // render
        {
            glClearColor(BLACK, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            /* float greenValue = sin(time) * 0.5 + 0.5; */

            shader.Bind();
            /* shader.setUniform4f("uColor", 0, greenValue, 0, 1); */

            texture1.Bind(0);
            texture2.Bind(1);

            // order of transformation is reversed (the last one is the first which is applied)
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(0.5f, 0.5f, 0.5f));
            /* model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f)); */ 
            /* model = glm::rotate(model, time, glm::vec3(1.0f, 1.0f, 1.0f)); */

            glm::mat4 view = glm::mat4(1.0f);
            // note that we're translating the scene in the reverse direction of where we want to move
            view = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f)); 
            
            glm::mat4 projection = glm::perspective(glm::radians(fov), ASPECT_RATIO, 0.1f, 100.0f);

            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); 

            // TODO: figure out if better to compute mvp matrix on cpu vs doing it in vertex shader
            shader.SetUniformMatrix4fv("model", model);
            shader.SetUniformMatrix4fv("view", view);
            shader.SetUniformMatrix4fv("projection", projection);

            glBindVertexArray(cubeVao.GetVaoId());
            glDrawElements(GL_TRIANGLES, cubeVao.GetCountIndices(), GL_UNSIGNED_INT, 0);

            shader2.Bind();
            texture2.Bind(0);
            model = glm::mat4(1.0f);
            view = glm::mat4(1.0f);
            view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); 

            shader2.SetUniformMatrix4fv("model", model);
            shader2.SetUniformMatrix4fv("view", view);
            shader2.SetUniformMatrix4fv("projection", projection);

            glBindVertexArray(tilesVao.GetVaoId());
            glDrawElements(GL_TRIANGLES, tilesVao.GetCountIndices(), GL_UNSIGNED_INT, 0);
        }

        SDL_GL_SwapWindow(window);
    }


    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
