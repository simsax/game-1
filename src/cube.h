#ifndef CUBE_H
#define CUBE_H

#include <vector>
#include "render.h"
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 texture;
};

class Cube {
public:
    Cube();
private:
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
};

#endif // CUBE_H
