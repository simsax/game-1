#include "vao.h"

uint32_t getSizeOfType(GLenum type) {
    switch (type) {
        case GL_DOUBLE:
            return 8;
        case GL_FLOAT:
        case GL_UNSIGNED_INT:
        case GL_INT:
            return 4;
        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            return 2;
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return 1;
        default:
            return 0;
    }
}


void Vao::Bind() {
    glBindVertexArray(m_Vao);
}

void Vao::Unbind() {
    glBindVertexArray(0);
}

uint32_t Vao::GetCountIndices() {
    return m_IndicesCount;
}

uint32_t Vao::GetVaoId() {
    return m_Vao;
}
