#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>
#include <glm/glm.hpp>

#define AXIS_RED    0xFB6666
#define AXIS_GREEN  0x66FB66
#define AXIS_BLUE   0x6666FB
#define BG_COLOR    0x222222
#define CAST_TILE   0xDDFB66
#define SELECT_TILE 0x003366
#define GROUND_TILE_COLOR 0xFFFFFF
#define DARK_TILE_COLOR 0x770000
#define LIGHT_TILE_COLOR 0xFF0000
#define TARGET_OFF_TILE_COLOR 0x007700
#define TARGET_ON_TILE_COLOR 0x00FF00


glm::vec3 hexToRgb(uint32_t color);
std::vector<uint32_t> generateQuadIndices(size_t numQuads);
std::vector<uint32_t> generateQuadLineIndices(size_t numQuads);

#endif // RENDER_H
