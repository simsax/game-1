#include "render.h"

glm::vec3 hexToRgb(uint32_t color) {
    return {
        ((color >> 16) & 0xFF) / 255.0f,
        ((color >> 8) & 0xFF)  / 255.0f,
        ((color >> 0) & 0xFF)  / 255.0f,
    };
}

std::vector<uint32_t> generateQuadIndices(size_t numQuads) {
    const size_t numVertices = numQuads * 4;
    const size_t numIndices = numQuads * 6;

    std::vector<uint32_t> indices;
    indices.reserve(numIndices);
    for (size_t i = 0; i < numVertices; i += 4) {
        indices.emplace_back(i + 0);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 3);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 3);
    }

    return indices;
}

std::vector<uint32_t> generateQuadLineIndices(size_t numQuads) {
    const size_t numVertices = numQuads * 4;
    const size_t numIndices = numQuads * 8;

    std::vector<uint32_t> indices;
    indices.reserve(numIndices);
    for (size_t i = 0; i < numVertices; i += 4) {
        indices.emplace_back(i + 0);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 1);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 2);
        indices.emplace_back(i + 3);
        indices.emplace_back(i + 3);
        indices.emplace_back(i + 0);
    }

    return indices;
}
