#ifndef RENDER_H
#define RENDER_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>

std::vector<uint32_t> generateQuadIndices(size_t numQuads);
std::vector<uint32_t> generateQuadLineIndices(size_t numQuads);

#endif // RENDER_H
