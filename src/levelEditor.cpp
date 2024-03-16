#include <fstream>
#include <iostream>
#include <filesystem>
#include "levelEditor.h"
#include "render.h"
#include "logger.h"
#include "imgui.h"
#include "shader.h"
#include "utils.h"

#define LEVEL_STR(levelNum) (std::format(ABS_PATH("/res/levels/level_{}.txt"), (levelNum) + 1).c_str())

enum class Face {
    U, F, D, B, L, R
};

struct LevelState {
    std::vector<TileType> tiles;
    glm::mat4 model;
    std::array<Face, 6> playerRot;
    glm::vec3 playerPos;
};

namespace levelEditor {

    // attributes
    Mesh editorGridMesh;
    Mesh selectedTilesMesh;
    Mesh castedTileMesh;
    Mesh axisLinesMesh;
    Shader editorGridShader;
    Shader selectedTilesShader;
    Shader axisShader;
    std::vector<TileQuad> gridVertices;
    std::vector<TileQuad> selectedTilesVertices;
    std::vector<uint32_t> gridIndices;
    std::vector<uint32_t> selectedTilesIndices;
    std::vector<int> selectedTiles;
    std::vector<LineVertex> axisLines;
    std::vector<Layout> layout = { { GL_FLOAT, 3 }, { GL_FLOAT, 3 } };
    TileQuad castedTileQuad;
    glm::vec3 axisOffset;
    int gridSideLength;
    int numTiles;
    int gridOffset;
    int castedTile = -1;
    bool selectionNeedsUpdate = true;
    TileType activeTileTypeButton = TileType::EMPTY_TILE;

    static int levelCounter = 0;
    static int currentLevel = -1;

    // functions
    void Init(int sideLength, const glm::vec3& lineColor, const char* vertexShaderPath,
            const char* fragmentShaderPath, const char* selectedFragmentShaderPath,
            const char* axisVertexShaderPath)
    {
        gridSideLength = sideLength;
        gridOffset = gridSideLength / 2;
        numTiles = sideLength * sideLength;
        static constexpr glm::vec3 selectedLinesColor = glm::vec3(0,0.7,1);

        // vertices
        {
            gridVertices.reserve(numTiles);
            selectedTiles.reserve(numTiles);
            gridIndices = generateQuadLineIndices(numTiles);
            selectedTilesIndices = generateQuadIndices(numTiles);

            // editor lines data
            for (int z = 0; z < sideLength; z++) {
                for (int x = 0; x < sideLength; x++) {
                    gridVertices.push_back({
                            Vertex{glm::vec3(x - gridOffset, 0.0f, z - gridOffset), lineColor},
                            Vertex{glm::vec3(x + 1 - gridOffset, 0.0f, z - gridOffset), lineColor},
                            Vertex{glm::vec3(x + 1 - gridOffset, 0.0f, z + 1 - gridOffset), lineColor},
                            Vertex{glm::vec3(x - gridOffset, 0.0f, z + 1 - gridOffset), lineColor}
                            });

                    selectedTilesVertices.push_back({
                            Vertex{glm::vec3(x - gridOffset, 0.01f, z - gridOffset), selectedLinesColor},
                            Vertex{glm::vec3(x + 1 - gridOffset, 0.01f, z - gridOffset), selectedLinesColor},
                            Vertex{glm::vec3(x + 1 - gridOffset, 0.01f, z + 1 - gridOffset), selectedLinesColor},
                            Vertex{glm::vec3(x - gridOffset, 0.01f, z + 1 - gridOffset), selectedLinesColor}
                            });
                }
            }

            castedTileQuad = {
                Vertex{glm::vec3(0), glm::vec3(0, 1, 0)},
                Vertex{glm::vec3(0), glm::vec3(0, 1, 0)},
                Vertex{glm::vec3(0), glm::vec3(0, 1, 0)},
                Vertex{glm::vec3(0), glm::vec3(0, 1, 0)},
            };

            // axis lines
            static constexpr float halfLength = 1000.0f;
            glm::vec3 axisRed = hexToRgb(AXIS_RED);
            glm::vec3 axisGreen = hexToRgb(AXIS_GREEN);
            glm::vec3 axisBlue = hexToRgb(AXIS_BLUE);
            axisOffset = glm::vec3(0);

            axisLines = {
                LineVertex({
                        Vertex{glm::vec3(-halfLength, 0, 0), axisRed},
                        Vertex{glm::vec3(halfLength, 0, 0), axisRed},
                        }),
                LineVertex({
                        Vertex{glm::vec3(0, 0, -halfLength), axisBlue},
                        Vertex{glm::vec3(0, 0, halfLength), axisBlue},
                        }),
                LineVertex({
                        Vertex{glm::vec3(0, -halfLength, 0), axisGreen},
                        Vertex{glm::vec3(0, halfLength, 0), axisGreen},
                        }),
            };
        }

        // meshes
        {
            editorGridMesh = Mesh(
                    gridVertices.data(),
                    gridVertices.size() * TileQuad::numVertices,
                    gridVertices.size() * sizeof(TileQuad),
                    layout,
                    GL_STATIC_DRAW,
                    gridIndices.data(),
                    gridIndices.size(),
                    gridIndices.size() * sizeof(uint32_t));

            selectedTilesMesh = Mesh(
                    selectedTilesVertices.data(),
                    selectedTilesVertices.size() * TileQuad::numVertices,
                    selectedTilesVertices.size() * sizeof(TileQuad),
                    layout,
                    GL_DYNAMIC_DRAW,
                    selectedTilesIndices.data(),
                    selectedTilesIndices.size(),
                    selectedTilesIndices.size() * sizeof(uint32_t));

            castedTileMesh = Mesh(nullptr, TileQuad::numVertices,
                    sizeof(TileQuad), layout, GL_DYNAMIC_DRAW);

            axisLinesMesh = Mesh(
                    axisLines.data(),
                    axisLines.size() * LineVertex::numVertices,
                    axisLines.size() * sizeof(LineVertex),
                    layout,
                    GL_STATIC_DRAW);
        }


        // shaders
        {
            editorGridShader = Shader(vertexShaderPath, fragmentShaderPath);
            axisShader = Shader(axisVertexShaderPath, fragmentShaderPath);
            selectedTilesShader = Shader(vertexShaderPath, selectedFragmentShaderPath);
        }

        // scan levels directory and update counter
        for (const auto& entry : std::filesystem::directory_iterator(ABS_PATH("/res/levels")))
            levelCounter++;
    }

    void Update() {
        // super inefficient but who cares, it's just the editor
        if (selectionNeedsUpdate) {
            selectionNeedsUpdate = false;
            std::vector<TileQuad> currentSelectedVertices;
            const int numSelected = selectedTiles.size();
            currentSelectedVertices.reserve(numSelected);

            for (int tileIx : selectedTiles) {
                currentSelectedVertices.push_back(selectedTilesVertices[tileIx]);
            }

            selectedTilesIndices = generateQuadIndices(numSelected);

            selectedTilesMesh.UpdateBufferData(0, currentSelectedVertices.size(),
                    currentSelectedVertices.size() * sizeof(TileQuad), currentSelectedVertices.data());
            selectedTilesMesh.UpdateElementBufferData(0, selectedTilesIndices.size(),
                    selectedTilesIndices.size() * sizeof(uint32_t), selectedTilesIndices.data());
        }
    }

    static void AddTiles(TileType tileType, std::vector<TileType>& tiles) {
        if (selectedTiles.size() > 0) {
            for (int tileIx : selectedTiles)
                tiles[tileIx] = tileType;
            selectedTiles.clear();
            selectionNeedsUpdate = true;
        }
    }

    static void ResetLevelState(LevelState& levelState) {
        std::fill(levelState.tiles.begin(), levelState.tiles.end(), TileType::EMPTY_TILE);
        levelState.model = glm::mat4({
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0.5, 0.5, 0.5, 1
                });
        levelState.playerRot = { Face::U, Face::F, Face::D, Face::B, Face::L, Face::R };
        levelState.playerPos = glm::vec3(0.0f);
    }

    void Render(const glm::mat4& mvp,
            bool& tilesNeedUpdate,
            LevelState& levelState) {
        // render tile grid
        editorGridShader.Bind();
        editorGridShader.SetUniformMatrix4fv("mvp", mvp);
        editorGridMesh.BindVao();
        glLineWidth(1);
        glDrawElements(GL_LINES, editorGridMesh.GetNumIndices(), GL_UNSIGNED_INT, 0);

        // render raycasted tile
        if (castedTile != -1) {
            castedTileMesh.BindVao();
            glLineWidth(4);
            glDisable(GL_DEPTH_TEST);
            glDrawArrays(GL_LINE_LOOP, 0, castedTileMesh.GetNumVertices());
            glEnable(GL_DEPTH_TEST);
        }

        // render axis
        axisShader.Bind();
        axisShader.SetUniformMatrix4fv("mvp", mvp);
        axisShader.SetUniform3f("axisOffset", axisOffset.x, axisOffset.y, axisOffset.z);
        axisLinesMesh.BindVao();
        glLineWidth(2);
        glDrawArrays(GL_LINES, 0, axisLinesMesh.GetNumVertices() - LineVertex::numVertices);

        // render selected tiles
        selectedTilesShader.Bind();
        selectedTilesShader.SetUniformMatrix4fv("mvp", mvp);
        selectedTilesMesh.BindVao();
        glLineWidth(4);
        glDrawElements(GL_TRIANGLES, selectedTilesIndices.size(), GL_UNSIGNED_INT, 0);

        // imgui
        {
            ImGui::Begin("Editor");

            // tiles buttons
            ImGui::SeparatorText("Edit tiles");
            if (ImGui::Button("Empty")) {
                AddTiles(TileType::EMPTY_TILE, levelState.tiles);
                tilesNeedUpdate = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Ground")) {
                AddTiles(TileType::GROUND_TILE, levelState.tiles);
                tilesNeedUpdate = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Dark")) {
                AddTiles(TileType::DARK_TILE, levelState.tiles);
                tilesNeedUpdate = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Light")) {
                AddTiles(TileType::LIGHT_TILE, levelState.tiles);
                tilesNeedUpdate = true;
            }

            ImGui::SeparatorText("Level");
            if (ImGui::Button("Reset")) {
                ResetLevelState(levelState);
                tilesNeedUpdate = true;
            }

            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                SaveCurrentLevel(levelState);
                tilesNeedUpdate = true;
            }

            ImGui::Separator();

            // level buttons
            for (int i = 0; i < levelCounter; i++) {
                if (i == currentLevel) {
                    ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.0f, 0.7f, 0.7f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.0f, 0.8f, 0.8f));
                }

                bool pressed = ImGui::Button(std::format("Level {}", i + 1).c_str());

                if ((int)i == currentLevel) {
                    ImGui::PopStyleColor(3);
                }

                if (pressed) {
                    currentLevel = i;
                    LoadLevelFromFile(LEVEL_STR(currentLevel) , levelState);
                    tilesNeedUpdate = true;
                }
            }

            if (ImGui::Button("+")) {
                currentLevel++;
                ResetLevelState(levelState);
                SaveCurrentLevel(levelState);
                tilesNeedUpdate = true;
            }

            ImGui::End();
        }
    }

    int GetTileIndex(int tileX, int tileZ) {
        // could use std::optional but at the moment I don't give a fuck
        tileX += gridOffset;
        tileZ += gridOffset;
        if (tileX >= 0 && tileZ >= 0 && tileX < gridSideLength && tileZ < gridSideLength)
            return tileZ * gridSideLength + tileX;
        else
            return -1;
    }

    void AddCastedToSelected() {
        selectedTiles.push_back(castedTile);
        selectionNeedsUpdate = true;
    }

    void RemoveCastedFromSelected() {
        std::erase(selectedTiles, castedTile);
        selectionNeedsUpdate = true;
    }

    void LoadLevelFromFile(const char* filePath, LevelState& levelState) {
        std::ifstream inputFile(filePath);

        if (!inputFile) {
            LOG_ERROR("Failed to read {}", filePath);
            exit(EXIT_FAILURE);
        }

        std::string line;
        int lineCounter = 0;
        int tileCounter = 0;
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
                            levelState.tiles[tileCounter++] = TileType::EMPTY_TILE;
                            break;
                        case '#':
                            levelState.tiles[tileCounter++] = TileType::GROUND_TILE;
                            break;
                        case 'D':
                            levelState.tiles[tileCounter++] = TileType::DARK_TILE;
                            break;
                        case 'L':
                            levelState.tiles[tileCounter++] = TileType::LIGHT_TILE;
                            break;
                        default:
                            break;
                    }
                }
            }
            lineCounter++;
        }
    }

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

    void SaveCurrentLevel(LevelState& levelState) {
        // feels weird to pass the levelState but at the same time it's more functional 
        // but in this case maybe having a global state here makes more sense 
        // than having it in main
        if (currentLevel != -1) {
            static constexpr int sideNum = 100;
            SaveLevelToFile(LEVEL_STR(currentLevel), levelState, sideNum);
        }
    }

}

