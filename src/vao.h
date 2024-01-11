#ifndef VAO_H
#define VAO_H

class Vao {
public:
    // can you derive vertex layout from the vertex struct? That would be sick
    Vao();
    bind();
    unbind();
private:
    uint32_t m_Vao;
    uint32_t m_Vbo;
    uint32_t m_Ebo;
};

#endif // VAO_H
