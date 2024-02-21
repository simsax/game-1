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
#define CAST_TILE   0xddfb66


glm::vec3 hexToRgb(uint32_t color);
std::vector<uint32_t> generateQuadIndices(size_t numQuads);
std::vector<uint32_t> generateQuadLineIndices(size_t numQuads);

#endif // RENDER_H
