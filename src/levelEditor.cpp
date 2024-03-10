#include "levelEditor.h"
#include "render.h"
#include "logger.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

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

    void Render(const glm::mat4& mvp,
            bool* tilesNeedUpdate,
            std::vector<TileType>& tiles) {
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

            if (ImGui::Button("Empty")) {
                AddTiles(TileType::EMPTY_TILE, tiles);
                *tilesNeedUpdate = true;
            }

            if (ImGui::Button("Ground")) {
                AddTiles(TileType::GROUND_TILE, tiles);
                *tilesNeedUpdate = true;
            }

            if (ImGui::Button("Dark")) {
                AddTiles(TileType::DARK_TILE, tiles);
                *tilesNeedUpdate = true;
            }

            if (ImGui::Button("Light")) {
                AddTiles(TileType::LIGHT_TILE, tiles);
                *tilesNeedUpdate = true;
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

}

