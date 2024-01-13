#ifndef TEXTURE_H
#define TEXTURE_H

#include <cstdint>
#include <glad/glad.h>

class Texture {
public:
    Texture(const char* path, GLenum internalFormat, GLenum externalFormat);
    void Bind(uint8_t unit);
private:
    uint32_t m_TextureId;
};

#endif // TEXTURE_H
