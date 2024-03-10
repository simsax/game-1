#include <SDL_keycode.h>
#include <SDL_mouse.h>
#include <SDL_stdinc.h>
#include <SDL_video.h>
#include <cmath>
#include <cstdlib>
#include <SDL2/SDL.h>
#include "render.h"
#include "utils.h"
#include "shader.h"
#include "mesh.h"
#include "camera.h"
#include "logger.h"
#include "levelEditor.h"
// imgui
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <fstream>
#include <iostream>

#define WIREFRAME 0

#ifndef NDEBUG
#define DEBUG
#endif

// this will be changed dynamically to allow resizing
#define SCREEN_WIDTH 1920
#define SCREEN_HEIGHT 1080


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

static void turn(std::array<Face, 6>& state, Rotation rotation) {
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

struct Tile {
    void SetColor(const glm::vec3& color) {
        for (auto& vertex : tileVertices) {
            vertex.color = color;
        }
    }

    static constexpr int numVertices = 4;
    Vertex tileVertices[numVertices]; // mesh
};

struct LevelState {
    std::vector<TileType> tiles;
    glm::mat4 model;
    std::array<Face, 6> playerRot;
    glm::vec3 playerPos;
} levelState = {
    {},
    glm::mat4(1.0f),
    { Face::U, Face::F, Face::D, Face::B, Face::L, Face::R },
    glm::vec3(0.0f)};

// game state globals
bool tilesNeedUpdate = true;
std::vector<Tile> currentGroundVertices;
bool firstAccess = false;

void SaveLevelToFile(const char* filePath, const LevelState& levelState, int rowLength) {
    std::ofstream outputFile(filePath);

    if (!outputFile) {
        LOG_ERROR("Failed at creating file {}", filePath);
        exit(EXIT_FAILURE);
    }

    // save player pos
    outputFile << "(" << levelState.playerPos[0] << ",";
    outputFile << levelState.playerPos[1] << ",";
    outputFile << levelState.playerPos[2] << ")\n";

    // save player orientation (cube state)
    outputFile << "(";
    for (int i = 0; i < 6; i++) {
        outputFile << (int)levelState.playerRot[i];
        if (i != 5)
            outputFile << ",";
    }
    outputFile << ")\n";

    // save player orientation (model matrix)
    outputFile << "[";
    for (int i = 0; i < 4; i++) {
        outputFile << "[";
        for (int j = 0; j < 4; j++) {
            outputFile << levelState.model[i][j];
            if (j != 3)
                outputFile << ",";
        }
        outputFile << "]";
        if (i != 3)
            outputFile << ",";
    }
    outputFile << "]\n";

    // save tiles map
    int colCounter = 0;
    for (auto tile : levelState.tiles) {
        switch (tile) {
            case TileType::EMPTY_TILE:
                outputFile << ".";
                break;
            case TileType::GROUND_TILE:
                outputFile << "#";
                break;
            case TileType::DARK_TILE:
                outputFile << "D";
                break;
            case TileType::LIGHT_TILE:
                outputFile << "L";
                break;
        }
        colCounter++;
        if (colCounter == rowLength) {
            outputFile << "\n";
            colCounter = 0;
        }
    }
}

LevelState LoadLevelFromFile(const char* filePath, size_t maxSize) {
    LevelState levelState;
    std::ifstream inputFile(filePath);
    levelState.tiles.reserve(maxSize);

    if (!inputFile) {
        LOG_ERROR("Failed to read {}", filePath);
        exit(EXIT_FAILURE);
    }

    std::string line;
    int lineCounter = 0;
    while (std::getline(inputFile, line)) {
        if (lineCounter == 0) {
            // load pos
            int numParsed = 0;
            size_t i = 0;
            while (i < line.length()) {
                char c = line[i];
                switch (c) {
                    case '(':
                    case ')':
                    case ',':
                    case ' ':
                        i++;
                        break;
                    default:
                        // parse number
                        {
                            size_t start = i;
                            while (i < line.length() - 1 &&
                                    line[i + 1] != ',' &&
                                    line[i + 1] != ')' &&
                                    line[i + 1] != ' ') {
                                i++;
                            }
                            size_t numLen = i - start + 1;
                            std::string numStr = line.substr(start, numLen);
                            if (numParsed > 2) {
                                LOG_ERROR("Error while loading level file, player position is invalid");
                            }
                            levelState.playerPos[numParsed++] = std::stof(numStr);
                            i++;
                        }
                        break;
                }
            }
        } else if (lineCounter == 1) {
            // load cubeState
            int numParsed = 0;
            size_t i = 0;
            while (i < line.length()) {
                char c = line[i];
                switch (c) {
                    case '(':
                    case ')':
                    case ',':
                    case ' ':
                        i++;
                        break;
                    default:
                        // parse number
                        {
                            size_t start = i;
                            while (i < line.length() - 1 &&
                                    line[i + 1] != ',' &&
                                    line[i + 1] != ')' &&
                                    line[i + 1] != ' ') {
                                i++;
                            }
                            size_t numLen = i - start + 1;
                            std::string numStr = line.substr(start, numLen);
                            if (numParsed > 5) {
                                LOG_ERROR("Error while loading level file, player rotation is invalid");
                            }
                            levelState.playerRot[numParsed++] = static_cast<Face>(std::stoi(numStr));
                            i++;
                        }
                        break;
                }
            }

        } else if (lineCounter == 2) {
            // load model matrix
            int numParsed = 0;
            size_t i = 0;
            while (i < line.length()) {
                char c = line[i];
                switch (c) {
                    case '[':
                    case ']':
                    case ',':
                    case ' ':
                        i++;
                        break;
                    default:
                        // parse number
                        {
                            size_t start = i;
                            while (i < line.length() - 1 &&
                                    line[i + 1] != ',' &&
                                    line[i + 1] != '[' &&
                                    line[i + 1] != ']' &&
                                    line[i + 1] != ' ') {
                                i++;
                            }
                            size_t numLen = i - start + 1;
                            std::string numStr = line.substr(start, numLen);
                            if (numParsed > 15) {
                                LOG_ERROR("Error while loading level file, model matrix is invalid");
                            }
                            int row = numParsed / 4;
                            int col = numParsed % 4;
                            levelState.model[row][col] = std::stof(numStr);
                            numParsed++;
                            i++;
                        }
                        break;
                }
            }
        } else {
            // load tile map
            for (char c : line) {
                switch (c) {
                    case '.':
                        levelState.tiles.push_back(TileType::EMPTY_TILE);
                        break;
                    case '#':
                        levelState.tiles.push_back(TileType::GROUND_TILE);
                        break;
                    case 'D':
                        levelState.tiles.push_back(TileType::DARK_TILE);
                        break;
                    case 'L':
                        levelState.tiles.push_back(TileType::LIGHT_TILE);
                        break;
                    default:
                        break;
                }
            }
        }
        lineCounter++;
    }

    return levelState;
}

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glLineWidth(1);
    // v sync (disable when profiling performance)
    SDL_GL_SetSwapInterval(1); // read the docs (can be 0, 1, -1)

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    /* io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls */
    /* io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls */
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    io.Fonts->AddFontFromFileTTF(ABS_PATH("/res/fonts/UbuntuMonoNerdFont-Regular.ttf"), 16);

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, &context);
    ImGui_ImplOpenGL3_Init();


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


    // level tiles
    static constexpr int sideNum = 100;
    static constexpr int offset = sideNum / 2;
    static constexpr int numTiles = sideNum * sideNum;
    std::vector<Tile> tilesVertices;
    tilesVertices.reserve(numTiles);
    if (!firstAccess)
        levelState = LoadLevelFromFile(ABS_PATH("/res/levels/level1.txt"), numTiles);
    currentGroundVertices.reserve(numTiles);

    for (int z = 0; z < sideNum; z++) {
        for (int x = 0; x < sideNum; x++) {
            tilesVertices.push_back({
                Vertex{glm::vec3(x - offset, 0.0f, z - offset),         glm::vec3(1.0f),  glm::vec2(0.0f, 0.0f)},
                Vertex{glm::vec3(x + 1 - offset, 0.0f, z - offset),     glm::vec3(1.0f),  glm::vec2(1.0f, 0.0f)},
                Vertex{glm::vec3(x + 1 - offset, 0.0f, z + 1 - offset), glm::vec3(1.0f),  glm::vec2(1.0f, 1.0f)},
                Vertex{glm::vec3(x - offset, 0.0f, z + 1 - offset),     glm::vec3(1.0f),  glm::vec2(0.0f, 1.0f)}
            });
        }
    }

    levelEditor::Init(sideNum, glm::vec3(0.5f), 
            ABS_PATH("/res/shaders/editorTileShader.vert"),
            ABS_PATH("/res/shaders/editorTileShader.frag"),
            ABS_PATH("/res/shaders/selectedTileShader.frag"),
            ABS_PATH("/res/shaders/axisShader.vert"));

    std::vector<uint32_t> cubeIndices = generateQuadIndices(6);
    std::vector<uint32_t> tilesIndices = generateQuadIndices(numTiles);

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
            tilesVertices.data(),
            tilesVertices.size() * Tile::numVertices,
            tilesVertices.size() * sizeof(Tile),
            tilesLayout,
            GL_DYNAMIC_DRAW,
            tilesIndices.data(),
            tilesIndices.size(),
            tilesIndices.size() * sizeof(uint32_t));


    // compile and link shaders
    Shader cubeShader = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/cubeShader.frag"));

    Shader tilesShader = Shader(
            ABS_PATH("/res/shaders/cubeShader.vert"),
            ABS_PATH("/res/shaders/tileShader.frag"));


    // some globals (outside the game loop)
    bool quit = false;
    static constexpr glm::vec3 cameraPos = glm::vec3(-50.0f, 50.0f, 50.0f);
    static const glm::vec3 cameraFront = - glm::normalize(cameraPos);
    // with such a small fov, perspective starts to look like an ortrographic projection 
    // since lines are almost parallel
    Camera camera = Camera(SCREEN_WIDTH, SCREEN_HEIGHT, 0.1f, 10000.0f, cameraPos, cameraFront,
            glm::vec3(0.0f, 1.0f, 0.0f), 10.0f, 10.0f, 0.1f, 20.0f);
    glm::vec3 absoluteTrans = glm::vec3(0.5f);

    if (firstAccess) {
        levelState.model = glm::translate(glm::mat4(1.0f), absoluteTrans);
        levelState.tiles = std::vector(numTiles, TileType::EMPTY_TILE);
    }

    glm::mat4 frozenModel = levelState.model;
    bool rotating = false;
    float angle = 0.0f;
    glm::vec3 axis = glm::vec3(0);
    glm::vec3 translationAxis = glm::vec3(0);
    float curAngle = 0.0f;
    float rotationSpeed = 10.0f;
    float t = 0.0f;
    Rotation rotation;
    bool editorMode = true; // maybe this will turn into an enum
    double deltaTime = 0; // time between current and last frame
    double lastTime = 0; // time of last frame
    double prevTime = 0; // start time of the current second
    int numFrames = 0;
    bool wheelPressed = false;
    bool shiftPressed = false;
    const glm::vec3 tileColor = hexToRgb(CAST_TILE);
    bool mouseOnUI = false;

    // game loop
    while(!quit) {
        mouseOnUI = io.WantCaptureMouse;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event); // Forward your event to backend
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
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Move(Direction::FORWARD);
                            else
                                camera.Move(Direction::UP);
                            break;
                        case SDLK_a:
                            camera.Move(Direction::LEFT);
                            break;
                        case SDLK_s:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Move(Direction::BACKWARDS);
                            else
                                camera.Move(Direction::DOWN);
                            break;
                        case SDLK_d:
                            camera.Move(Direction::RIGHT);
                            break;
                        case SDLK_SPACE:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Move(Direction::UP);
                            break;
                        case SDLK_LCTRL:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Move(Direction::DOWN);
                            break;
                        case SDLK_DOWN:
                            if (!rotating) {
                                axis = glm::vec3(1,0,0);
                                angle = 90.0f;
                                rotating = true;
                                frozenModel = levelState.model;
                                translationAxis = glm::vec3(0, 0.5, -0.5);
                                rotation = Rotation::DOWN;
                                turn(levelState.playerRot, rotation);
                            }
                            break;
                        case SDLK_UP:
                            if (!rotating) {
                                axis = glm::vec3(1,0,0);
                                angle = -90.0f;
                                rotating = true;
                                frozenModel = levelState.model;
                                translationAxis = glm::vec3(0, 0.5, 0.5);
                                rotation = Rotation::UP;
                                turn(levelState.playerRot, rotation);
                            }
                            break;
                        case SDLK_LEFT:
                            if (!rotating) {
                                axis = glm::vec3(0,0,1);
                                angle = 90.0f;
                                rotating = true;
                                frozenModel = levelState.model;
                                translationAxis = glm::vec3(0.5, 0.5, 0);
                                rotation = Rotation::LEFT;
                                turn(levelState.playerRot, rotation);
                            }
                            break;
                        case SDLK_RIGHT:
                            if (!rotating) {
                                axis = glm::vec3(0,0,1);
                                angle = -90.0f;
                                rotating = true;
                                frozenModel = levelState.model;
                                translationAxis = glm::vec3(-0.5, 0.5, 0);
                                rotation = Rotation::RIGHT;
                                turn(levelState.playerRot, rotation);
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
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Stop(Direction::FORWARD);
                            else
                                camera.Stop(Direction::UP);
                            break;
                        case SDLK_a:
                            camera.Stop(Direction::LEFT);
                            break;
                        case SDLK_s:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Stop(Direction::BACKWARDS);
                            else
                                camera.Stop(Direction::DOWN);
                            break;
                        case SDLK_d:
                            camera.Stop(Direction::RIGHT);
                            break;
                        case SDLK_SPACE:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Stop(Direction::UP);
                            break;
                        case SDLK_LCTRL:
                            if (camera.GetType() == CameraType::PERSPECTIVE)
                                camera.Stop(Direction::DOWN);
                            break;
                        case SDLK_m:
                            camera.FlipMode();
                            break;
                        case SDLK_c:
                            camera.FlipType();
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
                    if (!mouseOnUI) {
                        switch (event.button.button) {
                            case SDL_BUTTON_LEFT:
                                if (levelEditor::castedTile != -1) {
                                    levelEditor::AddCastedToSelected();
                                }
                                break;
                            case SDL_BUTTON_RIGHT:
                                if (levelEditor::castedTile != -1) {
                                    levelEditor::RemoveCastedFromSelected();
                                }
                                break;
                            case SDL_BUTTON_MIDDLE:
                                wheelPressed = true;
                                break;
                            default:
                                break;
                        }
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (!mouseOnUI) {
                        if (event.button.button == SDL_BUTTON_MIDDLE) {
                            wheelPressed = false;
                        }
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (!mouseOnUI) {
                        // retrieve x and y offset from mouse movement
                        int xoffset = event.motion.xrel;
                        int yoffset = -event.motion.yrel; // down is 1 for SDL but -1 for OpenGL coords
                        if (camera.GetMode() == CameraMode::FLY) {
                            camera.Turn(xoffset, yoffset);
                        } else {
                            if (wheelPressed) {
                                if (shiftPressed) {
                                    camera.Pan(xoffset, yoffset);
                                } else {
                                    camera.Orbit(xoffset, yoffset);
                                }
                            }
                        }
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    if (!mouseOnUI) {
                        camera.Zoom(event.wheel.y);
                    }
                    break;
                default:
                    break;
            }
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

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

        camera.Update(deltaTime);
        levelEditor::Update();

        if (tilesNeedUpdate) {
            tilesNeedUpdate = false;
            currentGroundVertices.clear();

            size_t numVisible = 0;
            for (size_t i = 0; i < levelState.tiles.size(); i++) {
                switch (levelState.tiles[i]) {
                    case TileType::GROUND_TILE:
                        currentGroundVertices.push_back(tilesVertices[i]);
                        currentGroundVertices[numVisible].SetColor(hexToRgb(GROUND_TILE_COLOR));
                        numVisible++;
                        break;
                    case TileType::DARK_TILE:
                        currentGroundVertices.push_back(tilesVertices[i]);
                        currentGroundVertices[numVisible].SetColor(hexToRgb(DARK_TILE_COLOR));
                        numVisible++;
                        break;
                    case TileType::LIGHT_TILE:
                        currentGroundVertices.push_back(tilesVertices[i]);
                        currentGroundVertices[numVisible].SetColor(hexToRgb(LIGHT_TILE_COLOR));
                        numVisible++;
                        break;
                    case TileType::EMPTY_TILE:
                        break;
                }
            }

            tilesIndices = generateQuadIndices(numVisible);

            tilesMesh.UpdateBufferData(0, currentGroundVertices.size(),
                    currentGroundVertices.size() * sizeof(Tile), currentGroundVertices.data());
            tilesMesh.UpdateElementBufferData(0, tilesIndices.size(),
                    tilesIndices.size() * sizeof(uint32_t), tilesIndices.data());
        }

        // raycast
        levelEditor::castedTile = -1;
        if (!mouseOnUI) {
            if (editorMode && camera.GetMode() == CameraMode::ORBIT) {
                int x, y;
                uint32_t mouseState = SDL_GetMouseState(&x, &y);
                glm::vec4 ndcHomo = glm::vec4(
                        2.0f * x / SCREEN_WIDTH - 1.0f, -2.0f * y / SCREEN_HEIGHT + 1.0f, -1.0f, 1.0f);

                glm::vec4 rayEye;
                glm::vec3 planeNormal = glm::vec3(0.0f, -1.0f, 0.0f);
                glm::vec3 rayWorld;
                glm::vec3 cameraPos;

                if (camera.GetType() == CameraType::PERSPECTIVE) {
                    rayEye = glm::inverse(camera.GetPerspectiveProjection()) * ndcHomo;
                    rayEye.z = -1.0f;
                    rayEye.w = 0.0f;

                    /* glm::vec4 rayWorld4 = glm::inverse(view) * rayEye; */
                    /* rayWorld = glm::normalize(glm::vec3(rayWorld4.x, rayWorld4.y, rayWorld4.z)); */
                    rayWorld = glm::normalize(glm::inverse(camera.GetView()) * rayEye);
                    cameraPos = camera.GetPos();
                } else {
                    float xOrthoOffset = ndcHomo.x * camera.GetOrthoWidth() / 2;
                    float yOrthoOffset = ndcHomo.y * camera.GetOrthoHeight() / 2;

                    // For orthographic projection, the ray direction is constant and does not depend on the mouse position.
                    rayWorld = camera.GetFront();
                    glm::vec3 right = glm::normalize(glm::cross(camera.GetFront(), camera.GetUp()));
                    glm::vec3 up = glm::normalize(glm::cross(right, camera.GetFront()));
                    cameraPos = camera.GetPos() + right * xOrthoOffset + up * yOrthoOffset;
                }
                float dot = glm::dot(rayWorld, planeNormal);
                if (dot >= 0.0f) {
                    float t = cameraPos.y / dot;
                    glm::vec3 raycastedPoint = cameraPos + rayWorld * t;

                    int tileX = floor(raycastedPoint.x);
                    int tileZ = floor(raycastedPoint.z);
                    int tileIx = levelEditor::GetTileIndex(tileX, tileZ);
                    if (tileIx != -1) {
                        // TODO: kinda ugly, refactor
                        levelEditor::castedTile = tileIx;
                        levelEditor::castedTileQuad = levelEditor::gridVertices[tileIx];
                        levelEditor::castedTileQuad.SetColor(tileColor);
                        levelEditor::castedTileMesh.UpdateBufferData(0, levelEditor::TileQuad::numVertices,
                                sizeof(levelEditor::TileQuad), &levelEditor::castedTileQuad);
                    }
                }
            }
        }

        glm::vec3 normalizedCameraPos = glm::normalize(camera.GetPos());
        levelEditor::axisOffset = 0.1f * normalizedCameraPos;

        // render
        const glm::vec3 background = hexToRgb(BG_COLOR);
        glClearColor(background.x, background.y, background.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // imgui dockspace
        /* ImGui::DockSpaceOverViewport(ImGui::GetMainViewport()); */
        /* ImGui::ShowDemoWindow(); // Show demo window! :) */

        /* ImGui::Begin("Viewport"); */
        /* ImGui::End(); */

        if (rotating) {
            t += deltaTime * rotationSpeed; // make it a fixed update maybe
            if (t >= 1.0f) {
                t = 1.0f;
                rotating = false;
            }
            curAngle = t * angle;
            glm::mat4 trans = glm::translate(glm::mat4(1.0), - absoluteTrans - levelState.playerPos + translationAxis);
            glm::mat4 transBack = glm::translate(glm::mat4(1.0), absoluteTrans + levelState.playerPos - translationAxis);
            levelState.model = transBack * glm::rotate(glm::mat4(1.0), glm::radians(curAngle), axis) * trans * frozenModel;
            if (!rotating) {
                curAngle = 0.0f;
                t = 0.0f;
                // update the position only after the rotation has finished
                switch (rotation) {
                    case Rotation::UP:
                        levelState.playerPos.z -= 1;
                        break;
                    case Rotation::DOWN:
                        levelState.playerPos.z += 1;
                        break;
                    case Rotation::LEFT:
                        levelState.playerPos.x -= 1;
                        break;
                    case Rotation::RIGHT:
                        levelState.playerPos.x += 1;
                        break;
                    default:
                        break;
                }
                Face downFace = levelState.playerRot[(int)Orientation::DOWN];
                if (downFace == Face::F) {
                    if (levelState.playerPos.x >= 0 - offset && levelState.playerPos.z >= 0 - offset && levelState.playerPos.x < sideNum - offset && levelState.playerPos.z < sideNum - offset) {
                        int tileIx = (levelState.playerPos.z + offset) * sideNum + levelState.playerPos.x + offset;
                        if (levelState.tiles[tileIx] == TileType::DARK_TILE) {
                            levelState.tiles[tileIx] = TileType::LIGHT_TILE;
                            tilesNeedUpdate = true;
                        }
                    }
                }
            }
        }

        const glm::mat4& projection = camera.GetType() == CameraType::PERSPECTIVE ?
            camera.GetPerspectiveProjection() : camera.GetOrthographicProjection();

        const glm::mat4& view = camera.GetView();

        glm::mat4 vp = projection * view;
    
        // render cube
        {
            glm::mat4 mvp = vp * levelState.model;
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
            // hack to allow the editor to access the level tiles
            levelEditor::Render(vp, &tilesNeedUpdate, levelState.tiles);
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // serialize level state to a file
    // need to save player position, orientation and map
    SaveLevelToFile(ABS_PATH("/res/levels/level1.txt"), levelState, sideNum);

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

