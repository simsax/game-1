#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <vector>
#include <cstddef>
#include <cstdio>

struct Layout {
    GLenum type;
    GLint count;
};

inline uint32_t getSizeOfType(GLenum type) {
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

class Mesh {
    /*
        Note to self: this class is meant to be used with an interleaved vertex buffer. This means that
        the vertex buffer should contain all the vertex data in one buffer. If
        you want to use tightly packed vertex buffers, you should have to bind each vertex buffer
        before the call to the glVertexAttribPointer function. This is because the glVertexAttribPointer
        function uses the currently bound GL_ARRAY_BUFFER to specify the vertex attribute data.

        As a reminder, this is what a VAO looks like in pseudo code:

        struct VAO{
            GL_INT element_array_binding; //IBO used for glDrawElements and friends
            char* label;//for debugging

            struct{//per attribute values
                bool enabled; //whether to use a VBO for it

                //corresponding to the values passed into glVertexAttribPointer call
                int size;
                unsigned int stride;
                GL_ENUM type;
                bool normalized;
                bool integer; //unconverted integers
                bool long; //double precision
                void* offset;
                int bufferBinding;//GL_ARRAY_BUFFER bound at time of glVertexAttribPointer call

                int attributeDiviser; //as used for instancing

            } attributes[MAX_VERTEX_ATTRIBS];
        };
    */
public:
    Mesh() :
        m_Vao(0),
        m_Vbo(0),
        m_Ebo(0),
        m_Stride(0),
        m_VerticesCount(0),
        m_IndicesCount(0)
    { }

    Mesh(const void* verticesData, size_t verticesCount, size_t verticesSize,
        const std::vector<Layout>& layouts, GLenum usage,
        const void* indicesData = nullptr, size_t indicesCount = 0, size_t indicesSize = 0) :
        m_Vao(0),
        m_Vbo(0),
        m_Ebo(0),
        m_Stride(verticesSize / verticesCount),
        m_VerticesCount(verticesCount),
        m_IndicesCount(indicesCount)
    {
        // create vao
        glGenVertexArrays(1, &m_Vao);
        glBindVertexArray(m_Vao);

        // create vbo
        glGenBuffers(1, &m_Vbo);
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferData(GL_ARRAY_BUFFER, verticesSize, verticesData, usage);

        GLint size = 0;
        glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);

        // specify vertex attributes
        uint64_t offset = 0;
        for (size_t i = 0; i < layouts.size(); i++) {
            GLenum type = layouts[i].type;
            GLint count = layouts[i].count;

            glVertexAttribPointer(i, count, type, GL_FALSE, m_Stride, (void*) offset);
            glEnableVertexAttribArray(i);

            offset += count * getSizeOfType(type);
        }

        // create ebo
        if (indicesData) {
            glGenBuffers(1, &m_Ebo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo);
            // TODO: figure out if dynamic/static/stream
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicesSize, indicesData, GL_STATIC_DRAW);
        }
    }

    inline void UpdateBufferData(size_t offset, size_t size, const void* data) {
        glBindBuffer(GL_ARRAY_BUFFER, m_Vbo);
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, data);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    inline void UpdateElementBufferData(size_t offset, size_t size, const void* data) {
        BindVao();
        /* glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Ebo); */
        // reminder: binding the EBO is not necessary because VAO has direct reference of EBO
        // on the other end, if I bind the EBO while the wrong VAO is bound, I will override the 
        // EBO of that bound VAO
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
    }


    inline void BindVao() {
        glBindVertexArray(m_Vao);
    }

    inline void UnbindVao() {
        glBindVertexArray(0);
    }

    inline size_t GetNumVertices() const {
        return m_VerticesCount;
    }

    inline size_t GetNumIndices() const {
        return m_IndicesCount;
    }

    inline uint32_t GetVaoId() const {
        return m_Vao;
    }

private:
    uint32_t m_Vao;
    uint32_t m_Vbo;
    uint32_t m_Ebo;
    size_t m_Stride;
    size_t m_VerticesCount;
    size_t m_IndicesCount;
};

#endif // MESH_H

