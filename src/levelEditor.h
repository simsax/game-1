#ifndef EDITOR_TILE_H
#define EDITOR_TILE_H

#include <vector>
#include "mesh.h"
#include <glm/glm.hpp>

enum class TileType {
    EMPTY_TILE,
    GROUND_TILE,
    DARK_TILE,
    LIGHT_TILE
};

struct LevelState;

namespace levelEditor {
    /* namespace { */
    /*     // private stuff visible only by the parent namespace (same thing as 'static') */
    /* } */

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

    struct TileQuad {
        void SetColor(const glm::vec3& color) {
            for (auto& vertex : tileVertices) {
                vertex.color = color;
            }
        }

        static constexpr int numVertices = 4;
        Vertex tileVertices[numVertices]; // mesh
    };

    struct LineVertex {
        static constexpr int numVertices = 2;
        Vertex tileVertices[numVertices]; // mesh
    };

    void Init(int sideLength, const glm::vec3& lineColor, const char* vertexShaderPath,
            const char* fragmentShaderPath, const char* selectedFragmentShaderPath,
            const char* axisVertexShaderPath);
    void GetSideLength();
    int GetTileIndex(int tileX, int tileZ);
    void AddCastedToSelected();
    void RemoveCastedFromSelected();
    void Update();
    void Render(const glm::mat4& mvp, bool& tilesNeedUpdate, LevelState& levelState); 
    void LoadLevelFromFile(const char* path, LevelState& levelState);
    void SaveLevelToFile(const char* filePath, const LevelState& levelState, int rowLength);
    void SaveCurrentLevel(LevelState& levelState);

    extern Mesh castedTileMesh;
    extern TileQuad castedTileQuad;
    extern int castedTile;
    extern std::vector<TileQuad> gridVertices;
    extern glm::vec3 axisOffset;
    extern TileType activeTileTypeButton;
}

#endif // EDITOR_TILE_H
