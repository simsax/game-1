#include "render.h"

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
