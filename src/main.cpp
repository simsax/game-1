#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_stdinc.h>
#include <SDL_video.h>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <glm/gtx/string_cast.hpp>
#include "render.h"
#include "Config.h"
#include "utils.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "logger.h"

#define WIREFRAME 0

#ifndef NDEBUG
#define DEBUG
#endif

// this will be changed dynamically to allow resizing
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080

#define ABS_PATH(x) (PROJECT_SOURCE_DIR x)

#define SDL_ERROR() LOG_ERROR("SDL_Error: {}", SDL_GetError())

enum class Rotation {
    DOWN,
    UP,
    LEFT,
    RIGHT
};

enum class Orientation {
    UP,
    FRONT,
    DOWN,
    BACK,
    LEFT,
    RIGHT
};

enum class Face {
    U, F, D, B, L, R
};

Face cubeState[] = { Face::U, Face::F, Face::D, Face::B, Face::L, Face::R };

static void turn(Face* state, Rotation rotation) {
    switch (rotation) {
        case Rotation::DOWN:
            {
                Face temp = state[0];
                state[0] = state[3];
                state[3] = state[2];
                state[2] = state[1];
                state[1] = temp;
            }
            break;
        case Rotation::UP:
            {
                Face temp = state[0];
                state[0] = state[1];
                state[1] = state[2];
                state[2] = state[3];
                state[3] = temp;
            }
            break;
        case Rotation::LEFT:
            {
                Face temp = state[0];
                state[0] = state[5];
                state[5] = state[2];
                state[2] = state[4];
                state[4] = temp;
            }
            break;
        case Rotation::RIGHT:
            {
                Face temp = state[0];
                state[0] = state[4];
                state[4] = state[2];
                state[2] = state[5];
                state[5] = temp;
            }
            break;
        default:
            break;
    }
}


static const char* getFaceStr(Face face) {
    switch (face) {
        case Face::U:
            return "U";
        case Face::L:
            return "L";
        case Face::R:
            return "R";
        case Face::F:
            return "F";
        case Face::B:
            return "B";
        case Face::D:
            return "D";
    }
}

template <typename T>
T* SDL(T* ptr) {
    if (ptr == nullptr) {
        SDL_ERROR();
        exit(EXIT_FAILURE);
    }
    return ptr;
}

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture; // Probably I can cut this
};

struct EditorVertex {
    glm::vec3 position;
    glm::vec3 color;
};

struct LineVertex {
    static constexpr int numVertices = 2;
    EditorVertex tileVertices[numVertices]; // mesh
};

struct Tile {
    void SetColor(const glm::vec3& color) {
        for (auto& vertex : tileVertices) {
            vertex.color = color;
        }
    }

    static constexpr int numVertices = 4;
    Vertex tileVertices[numVertices]; // mesh
};

struct EditorTile {
    void SetColor(const glm::vec3& color) {
        for (auto& vertex : tileVertices) {
            vertex.color = color;
        }
    }

    static constexpr int numVertices = 4;
    EditorVertex tileVertices[numVertices]; // mesh
};

// TODO: face culling
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
    // set multi sampling antialiasing (MSAA)
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
#ifdef DEBUG
    LOG_DEBUG("DEBUG ON");
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
#if !ORTHO
    SDL_SetRelativeMouseMode(SDL_TRUE);
#endif

    SDL_GLContext context = SDL(SDL_GL_CreateContext(window));

    gladLoadGLLoader((GLADloadproc) SDL_GL_GetProcAddress);

    // opengl loaded
#ifdef DEBUG
    int flags;
    glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
    if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
    {
        // initialize debug output 
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
    }
#endif

#if WIREFRAME
    glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
#endif
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE); 
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2);
    // v sync (disable when profiling performance)
    SDL_GL_SetSwapInterval(1); // read the docs (can be 0, 1, -1)

    std::vector<Vertex> cubeVertices = {
        // up
        {glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f,  0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, 0.5f, -0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 0.0f)},

        // front
        {glm::vec3(0.5f,   0.5f, 0.5f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  -0.5f, 0.5f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, 0.5f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, 0.5f),  glm::vec3(1.0f, 0.0f, 0.0f),  glm::vec2(0.0f, 1.0f)},

        // down
        {glm::vec3(0.5f,  -0.5f,  0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f,  0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 1.0f)},

        // back
        {glm::vec3(0.5f,   0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f,  -0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 0.0f)},

        // left
        {glm::vec3(-0.5f,  0.5f,  0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(-0.5f,  -0.5f, 0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 0.0f)},
        {glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(-0.5f,  0.5f, -0.5f), glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 1.0f)},

        // right
        {glm::vec3(0.5f,  0.5f,  0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 1.0f)},
        {glm::vec3(0.5f,  0.5f, -0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 1.0f)},
        {glm::vec3(0.5f, -0.5f, -0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(0.0f, 0.0f)},
        {glm::vec3(0.5f, -0.5f,  0.5f),  glm::vec3(0.3f, 0.3f, 0.3f),  glm::vec2(1.0f, 0.0f)},
    };


    static constexpr int sideNum = 3;
    static constexpr int numTiles = sideNum * sideNum;
    std::vector<Tile> tilesVector;
    tilesVector.reserve(numTiles);

    for (int z = 0; z < sideNum; z++) {
        for (int x = 0; x < sideNum; x++) {
            tilesVector.push_back({
                Vertex{glm::vec3(x, 0.0f, z),         glm::vec3(1.0f),  glm::vec2(0.0f, 0.0f)},
                Vertex{glm::vec3(x + 1, 0.0f, z),     glm::vec3(1.0f),  glm::vec2(1.0f, 0.0f)},
                Vertex{glm::vec3(x + 1, 0.0f, z + 1), glm::vec3(1.0f),  glm::vec2(1.0f, 1.0f)},
                Vertex{glm::vec3(x, 0.0f, z + 1),     glm::vec3(1.0f),  glm::vec2(0.0f, 1.0f)}
            });
        }
    }

    static constexpr int sideNumEditor = 100;
    static constexpr int numTilesEditor = sideNumEditor * sideNumEditor;
    std::vector<EditorTile> editorTilesVector;
    editorTilesVector.reserve(numTilesEditor);

    static constexpr int offset = sideNumEditor / 2;
    for (int z = 0; z < sideNumEditor; z++) {
        for (int x = 0; x < sideNumEditor; x++) {
            editorTilesVector.push_back({
                    EditorVertex{glm::vec3(x - offset, 0.0f, z - offset), glm::vec3(1.0f)},
                    EditorVertex{glm::vec3(x + 1 - offset, 0.0f, z - offset), glm::vec3(1.0f)},
                    EditorVertex{glm::vec3(x + 1 - offset, 0.0f, z + 1 - offset), glm::vec3(1.0f)},
                    EditorVertex{glm::vec3(x - offset, 0.0f, z + 1 - offset), glm::vec3(1.0f)}
            });
        }
    }

    std::vector<uint32_t> cubeIndices = generateQuadIndices(6);
    std::vector<uint32_t> tilesIndices = generateQuadIndices(numTiles);
    std::vector<uint32_t> editorTilesIndices = generateQuadLineIndices(numTilesEditor);

    EditorTile castedTile = {
        EditorVertex{glm::vec3(0), glm::vec3(0, 1, 0)},
        EditorVertex{glm::vec3(0), glm::vec3(0, 1, 0)},
        EditorVertex{glm::vec3(0), glm::vec3(0, 1, 0)},
        EditorVertex{glm::vec3(0), glm::vec3(0, 1, 0)},
    };

    static constexpr float halfLength = 1000.0f;
    std::vector<LineVertex> axisLines = {
        LineVertex({
                EditorVertex{glm::vec3(-halfLength, 0, 0), glm::vec3(1,0,0)},
                EditorVertex{glm::vec3(halfLength, 0, 0), glm::vec3(1,0,0)},
                }),
        LineVertex({
                EditorVertex{glm::vec3(0, -halfLength, 0), glm::vec3(0,1,0)},
                EditorVertex{glm::vec3(0, halfLength, 0), glm::vec3(0,1,0)},
                }),
        LineVertex({
                EditorVertex{glm::vec3(0, 0, -halfLength), glm::vec3(0,0,1)},
                EditorVertex{glm::vec3(0, 0, halfLength), glm::vec3(0,0,1)},
                }),
    };

    std::vector<Layout> cubeLayout = {
        { GL_FLOAT, 3 },
        { GL_FLOAT, 3 },
        { GL_FLOAT, 2 }
    };

    std::vector<Layout> tilesLayout = {
        { GL_FLOAT, 3 },
        { GL_FLOAT, 3 },
        { GL_FLOAT, 2 }
    };

    std::vector<Layout> editorTilesLayout = {
        { GL_FLOAT, 3 },
        { GL_FLOAT, 3 }
    };

    Mesh cubeMesh = Mesh(
            cubeVertices.data(),
            cubeVertices.size(),
            cubeVertices.size() * sizeof(Vertex),
            cubeLayout,
            GL_STATIC_DRAW,
            cubeIndices.data(),
            cubeIndices.size(),
            cubeIndices.size() * sizeof(uint32_t));
    
    // TODO: dynamic or stream?
    Mesh tilesMesh = Mesh(
            tilesVector.data(),
            tilesVector.size() * Tile::numVertices,
            tilesVector.size() * sizeof(Tile),
            tilesLayout,
            GL_DYNAMIC_DRAW,
            tilesIndices.data(),
            tilesIndices.size(),
            tilesIndices.size() * sizeof(uint32_t));


    Mesh castedTileMesh = Mesh(&castedTile, EditorTile::numVertices, sizeof(EditorTile), tilesLayout, GL_DYNAMIC_DRAW);

    Mesh editorTilesMesh = Mesh(
            editorTilesVector.data(),
            editorTilesVector.size() * EditorTile::numVertices,
            editorTilesVector.size() * sizeof(EditorTile),
            editorTilesLayout,
            GL_STATIC_DRAW,
            editorTilesIndices.data(),
            editorTilesIndices.size(),
            editorTilesIndices.size() * sizeof(uint32_t));

    Mesh axisLinesMesh = Mesh(
            axisLines.data(),
            axisLines.size() * LineVertex::numVertices,
            axisLines.size() * sizeof(LineVertex),
            editorTilesLayout,
            GL_STATIC_DRAW);

    // compile and link shaders
    Shader cubeShader = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/cubeShader.frag"));

    Shader tilesShader = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/tileShader.frag"));

    Shader editorTilesShader = Shader(
            ABS_PATH("/res/shaders/editorTileShader.vert"),
            ABS_PATH("/res/shaders/editorTileShader.frag"));

    // some globals (outside the game loop)
    bool quit = false;
    bool mouseCaptured = false;
    CameraType cameraType = CameraType::ORTHOGRAPHIC;
    static constexpr float zoom = 10.0f;
    static constexpr glm::vec3 orthoPos = glm::vec3(-20.0f, 20.0f,  20.0f);
    static const glm::vec3 orthoFront = - glm::normalize(orthoPos);
    Camera orthoCam = Camera<CameraType::ORTHOGRAPHIC>(
            SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100.0f,
            orthoPos,
            orthoFront,
            glm::vec3(0.0f, 1.0f, 0.0f),
            10.0f,
            70.0f,
            0.1f,
            zoom);
    Camera perspectiveCam = Camera<CameraType::PERSPECTIVE>(
            SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 100.0f,
            glm::vec3(0.0f, 0.0f,  3.0f),
            glm::vec3(0.0f, 0.0f, -1.0f),
            glm::vec3(0.0f, 1.0f, 0.0f),
            10.0f,
            70.0f,
            0.1f);
    glm::vec3 pos = glm::vec3(0.0f);
    glm::vec3 absoluteTrans = glm::vec3(0.5f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), absoluteTrans);
    glm::mat4 frozenModel = model;
    bool rotating = false;
    float angle = 0.0f;
    glm::vec3 axis = glm::vec3(0);
    glm::vec3 translationAxis = glm::vec3(0);
    float curAngle = 0.0f;
    float rotationSpeed = 10.0f;
    float t = 0.0f;
    Rotation rotation;
    bool editorMode = false; // maybe this will turn into an enum
    double deltaTime = 0; // time between current and last frame
    double lastTime = 0; // time of last frame
    double prevTime = 0; // start time of the current second
    int numFrames = 0;
    bool wheelPressed = false;
    bool shiftPressed = false;

    // game loop
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
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::FORWARD);
                            else
                                orthoCam.Move(Direction::UP);
                            break;
                        case SDLK_a:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::LEFT);
                            else
                                orthoCam.Move(Direction::LEFT);
                            break;
                        case SDLK_s:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::BACKWARDS);
                            else
                                orthoCam.Move(Direction::DOWN);
                            break;
                        case SDLK_d:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::RIGHT);
                            else
                                orthoCam.Move(Direction::RIGHT);
                            break;
                        case SDLK_SPACE:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::UP);
                            break;
                        case SDLK_LCTRL:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Move(Direction::DOWN);
                            break;
                        case SDLK_DOWN:
                            if (!rotating) {
                                axis = glm::vec3(1,0,0);
                                angle = 90.0f;
                                rotating = true;
                                frozenModel = model;
                                translationAxis = glm::vec3(0, 0.5, -0.5);
                                rotation = Rotation::DOWN;
                                turn(cubeState, rotation);
                            }
                            break;
                        case SDLK_UP:
                            if (!rotating) {
                                axis = glm::vec3(1,0,0);
                                angle = -90.0f;
                                rotating = true;
                                frozenModel = model;
                                translationAxis = glm::vec3(0, 0.5, 0.5);
                                rotation = Rotation::UP;
                                turn(cubeState, rotation);
                            }
                            break;
                        case SDLK_LEFT:
                            if (!rotating) {
                                axis = glm::vec3(0,0,1);
                                angle = 90.0f;
                                rotating = true;
                                frozenModel = model;
                                translationAxis = glm::vec3(0.5, 0.5, 0);
                                rotation = Rotation::LEFT;
                                turn(cubeState, rotation);
                            }
                            break;
                        case SDLK_RIGHT:
                            if (!rotating) {
                                axis = glm::vec3(0,0,1);
                                angle = -90.0f;
                                rotating = true;
                                frozenModel = model;
                                translationAxis = glm::vec3(-0.5, 0.5, 0);
                                rotation = Rotation::RIGHT;
                                turn(cubeState, rotation);
                            }
                            break;
                        case SDLK_LSHIFT:
                            shiftPressed = true;
                            break;
                        default:
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_w:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::FORWARD);
                            else
                                orthoCam.Stop(Direction::UP);
                            break;
                        case SDLK_a:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::LEFT);
                            else
                                orthoCam.Stop(Direction::LEFT);
                            break;
                        case SDLK_s:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::BACKWARDS);
                            else
                                orthoCam.Stop(Direction::DOWN);
                            break;
                        case SDLK_d:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::RIGHT);
                            else
                                orthoCam.Stop(Direction::RIGHT);
                            break;
                        case SDLK_SPACE:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::UP);
                            break;
                        case SDLK_LCTRL:
                            if (cameraType == CameraType::PERSPECTIVE)
                                perspectiveCam.Stop(Direction::DOWN);
                            break;
                        case SDLK_m:
                            mouseCaptured = !mouseCaptured;
                            break;
                        case SDLK_c:
                            {
                                cameraType = cameraType == CameraType::PERSPECTIVE ?
                                    CameraType::ORTHOGRAPHIC : CameraType::PERSPECTIVE;

                                // uncapture mouse by default when in ortographic mode
                                if (cameraType == CameraType::ORTHOGRAPHIC)
                                    mouseCaptured = false;
                                else
                                    mouseCaptured = true;
                            }
                            break;
                        case SDLK_e:
                            editorMode = !editorMode;
                            break;
                        case SDLK_LSHIFT:
                            shiftPressed = false;
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
                    if (event.button.button == SDL_BUTTON_MIDDLE) {
                        wheelPressed = true;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    /* if (event.button.button == SDL_BUTTON_LEFT) { */
                    /*     mouse_up(LEFT_BUTTON); */
                    /*     undrag(); */
                    /* } else if (event.button.button == SDL_BUTTON_RIGHT) { */
                    /*     mouse_up(RIGHT_BUTTON); */
                    /* } */
                    if (event.button.button == SDL_BUTTON_MIDDLE) {
                        wheelPressed = false;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    {
                        // retrieve x and y offset from mouse movement
                        int xoffset = event.motion.xrel;
                        int yoffset = -event.motion.yrel; // down is 1 for SDL but -1 for OpenGL coords
                        if (cameraType == CameraType::PERSPECTIVE) {
                            if (mouseCaptured) {
                                perspectiveCam.Turn(xoffset, yoffset);
                            }
                            else {
                                if (wheelPressed) {
                                    if (shiftPressed) {
                                        // pan
                                        perspectiveCam.Pan(xoffset, yoffset);
                                    } else {
                                        // turn
                                        perspectiveCam.Orbit(xoffset, yoffset);
                                    }
                                }
                            }
                        } else { // orthographic
                            if (wheelPressed) {
                                if (shiftPressed) {
                                    // pan
                                    orthoCam.Pan(xoffset, yoffset);
                                } else {
                                    // turn
                                    /* orthoCam.Turn(xoffset, yoffset); */
                                    glm::vec3 target = glm::vec3(0);
                                    orthoCam.Orbit(xoffset, yoffset);
                                }
                            }
                        }
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    {
                        if (cameraType == CameraType::PERSPECTIVE)
                            perspectiveCam.Zoom(event.wheel.y);
                        else
                            orthoCam.Zoom(event.wheel.y);
                    }
                    break;
                default:
                    break;
            }
        }

        if (mouseCaptured)
            SDL_SetRelativeMouseMode(SDL_TRUE);
        else
            SDL_SetRelativeMouseMode(SDL_FALSE);

        // update
        double time = SDL_GetTicks() / 1000.0;
        deltaTime = time - lastTime;
        lastTime = time;
        numFrames++;

        if (lastTime - prevTime >= 1) {
            // LOG_INFO("FPS: {} / ms: {:.3}", numFrames, 1000.0f / numFrames);
            numFrames = 0;
            prevTime = lastTime;
        }

        // camera update
        if (cameraType == CameraType::PERSPECTIVE)
            perspectiveCam.Update(deltaTime);
        else
            orthoCam.Update(deltaTime);

        // raycast
        bool tileIsCasted = false;
        if (editorMode && !mouseCaptured) {
            int x, y;
            uint32_t mouseState = SDL_GetMouseState(&x, &y);
            glm::vec4 ndcHomo = glm::vec4(
                    2.0f * x / SCREEN_WIDTH - 1.0f, -2.0f * y / SCREEN_HEIGHT + 1.0f, -1.0f, 1.0f);

            glm::vec4 rayEye;
            glm::vec3 planeNormal = glm::vec3(0.0f, -1.0f, 0.0f);
            glm::vec3 rayWorld;
            glm::vec3 cameraPos;

            if (cameraType == CameraType::PERSPECTIVE) {
                rayEye = glm::inverse(perspectiveCam.GetProjection()) * ndcHomo;
                rayEye.z = -1.0f;
                rayEye.w = 0.0f;

                /* glm::vec4 rayWorld4 = glm::inverse(view) * rayEye; */
                /* rayWorld = glm::normalize(glm::vec3(rayWorld4.x, rayWorld4.y, rayWorld4.z)); */
                rayWorld = glm::normalize(glm::inverse(perspectiveCam.GetView()) * rayEye);
                cameraPos = perspectiveCam.GetPos();
            } else {
                float xOrthoOffset = ndcHomo.x * orthoCam.GetOrthoWidth() / 2;
                float yOrthoOffset = ndcHomo.y * orthoCam.GetOrthoHeight() / 2;

                // For orthographic projection, the ray direction is constant and does not depend on the mouse position.
                rayWorld = orthoCam.GetFront();
                glm::vec3 right = glm::normalize(glm::cross(orthoCam.GetFront(), orthoCam.GetUp()));
                glm::vec3 up = glm::normalize(glm::cross(right, orthoCam.GetFront()));
                cameraPos = orthoCam.GetPos() + right * xOrthoOffset + up * yOrthoOffset;
            }
            float dot = glm::dot(rayWorld, planeNormal);
            if (dot >= 0.0f) {
                float t = cameraPos.y / dot;
                glm::vec3 raycastedPoint = cameraPos + rayWorld * t;

                int tileX = floor(raycastedPoint.x);
                int tileZ = floor(raycastedPoint.z);
                tileX += offset;
                tileZ += offset;

                int tileIx = tileZ * sideNumEditor + tileX;
                if (tileX >= 0 && tileZ >= 0 && tileX < sideNumEditor && tileZ < sideNumEditor) {
                    tileIsCasted = true;
                    castedTile = editorTilesVector[tileIx];
                    castedTile.SetColor(glm::vec3(0, 1, 0));
                    castedTileMesh.UpdateBufferData(0, sizeof(EditorTile), &castedTile);
                }
            }
        }

        // update color only when hovered tile changes, not every frame

        // render
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (rotating) {
            t += deltaTime * rotationSpeed; // make it a fixed update maybe
            if (t >= 1.0f) {
                t = 1.0f;
                rotating = false;
            }
            curAngle = t * angle;
            glm::mat4 trans = glm::translate(glm::mat4(1.0), - absoluteTrans - pos + translationAxis);
            glm::mat4 transBack = glm::translate(glm::mat4(1.0), absoluteTrans + pos - translationAxis);
            model = transBack * glm::rotate(glm::mat4(1.0), glm::radians(curAngle), axis) * trans * frozenModel;
            if (!rotating) {
                curAngle = 0.0f;
                t = 0.0f;
                // update the position only after the rotation has finished
                switch (rotation) {
                    case Rotation::UP:
                        pos.z -= 1;
                        break;
                    case Rotation::DOWN:
                        pos.z += 1;
                        break;
                    case Rotation::LEFT:
                        pos.x -= 1;
                        break;
                    case Rotation::RIGHT:
                        pos.x += 1;
                        break;
                    default:
                        break;
                }
                Face downFace = cubeState[(int)Orientation::DOWN];
                if (downFace == Face::F) {
                    int tileIx = pos.z * sideNum + pos.x;
                    if (pos.x >= 0 && pos.z >= 0 && pos.x < sideNum && pos.z < sideNum) {
                        tilesVector[tileIx].SetColor({1, 0, 0});
                        tilesMesh.UpdateBufferData(tileIx * sizeof(Tile), sizeof(Tile), &tilesVector[tileIx]);
                        LOG_INFO("Red face is down, replacing tile at index: {}", tileIx);
                    }
                }
            }
        }

        const glm::mat4& projection = cameraType == CameraType::PERSPECTIVE ? perspectiveCam.GetProjection()
            : orthoCam.GetProjection();

        const glm::mat4& view = cameraType == CameraType::PERSPECTIVE ? perspectiveCam.GetView()
            : orthoCam.GetView();

        glm::mat4 vp = projection * view;
    
        // render cube
        {
            glm::mat4 mvp = vp * model;
            cubeShader.Bind();
            cubeShader.SetUniformMatrix4fv("mvp", mvp);

            cubeMesh.BindVao();
            glDrawElements(GL_TRIANGLES, cubeMesh.GetNumIndices(), GL_UNSIGNED_INT, 0);
        }

        // render tiles
        {
            tilesShader.Bind();
            tilesShader.SetUniformMatrix4fv("mvp", vp);

            tilesMesh.BindVao();
            glDrawElements(GL_TRIANGLES, tilesMesh.GetNumIndices(), GL_UNSIGNED_INT, 0);
        }

        // render level editor 
        if (editorMode) {
            editorTilesShader.Bind();
            editorTilesShader.SetUniformMatrix4fv("mvp", vp);

            editorTilesMesh.BindVao();
            glDrawElements(GL_LINES, editorTilesMesh.GetNumIndices(), GL_UNSIGNED_INT, 0);

            // render axis
            axisLinesMesh.BindVao();
            glLineWidth(4);
            glDrawArrays(GL_LINES, 0, axisLinesMesh.GetNumVertices());
            glLineWidth(2);

            // render raycasted tile
            if (tileIsCasted) {
                castedTileMesh.BindVao();
                glLineWidth(8);
                glDisable(GL_DEPTH_TEST);
                glDrawArrays(GL_LINE_LOOP, 0, castedTileMesh.GetNumVertices());
                glLineWidth(2);
                glEnable(GL_DEPTH_TEST);
            }

        }


        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

