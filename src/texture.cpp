#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "texture.h"
#include "logger.h"

Texture::Texture(const char* path, GLenum internalFormat, GLenum externalFormat) : m_TextureId(0) {
    glGenTextures(1, &m_TextureId);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
    // texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // texture filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image
    int width, height, nrChannels;
	// flip texture upside down because for opengl bottom left is the starting position
	stbi_set_flip_vertically_on_load(true);
    uint8_t *data = stbi_load(path, &width, &height, &nrChannels, 0); 
    if (data == nullptr) {
        LOG_ERROR("Failed at loading image: {}", path);
        exit(EXIT_FAILURE);
    }
    stbi_image_free(data);

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, externalFormat, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
}

void Texture::Bind(uint8_t unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, m_TextureId);
}
