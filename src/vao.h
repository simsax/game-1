#ifndef VAO_H
#define VAO_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>
#include <cstdio>

struct Layout {
    GLenum type;
    GLint count;
};

uint32_t getSizeOfType(GLenum type);

class Vao {
public:
    // TODO: figure out if I should switch to arrays (profile)
    template <typename T>
    inline Vao(const std::vector<T>& vertices, const std::vector<uint32_t>& indices,
            const std::vector<Layout>& layouts, GLenum usage):
        m_Vao(0), m_Vbo(0), m_Ebo(0), m_Stride(0), m_IndicesCount(indices.size())
    {
        // create vao
        glGenVertexArrays(1, &m_Vao);
        glBindVertexArray(m_Vao);

        // create vbo
        glGenBuffers(1, &m_Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(T), vertices.data(), usage);

        // create ebo
        glGenBuffers(1, &m_Ebo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
        // TODO: figure out if dynamic/static/stream
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(T), indices.data(), GL_STATIC_DRAW);

        // calculate stride
        for (size_t i = 0; i < layouts.size(); i++) {
            m_Stride += layouts[i].count * getSizeOfType(layouts[i].type);
        }

        // specify vertex attributes
        uint64_t offset = 0;
        for (size_t i = 0; i < layouts.size(); i++) {
            GLenum type = layouts[i].type;
            GLint count = layouts[i].count;

            glVertexAttribPointer(i, count, type, GL_FALSE, m_Stride, (void*) offset);
            glEnableVertexAttribArray(i);

            offset += count * getSizeOfType(type);
        }
    }

    void bind();
    void unbind();
    uint32_t getCountIndices();
    uint32_t getVaoId();
private:
    uint32_t m_Vao;
    uint32_t m_Vbo;
    uint32_t m_Ebo;
    int m_Stride;
    uint32_t m_IndicesCount;
};


#endif // VAO_H
