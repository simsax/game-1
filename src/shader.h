#ifndef SHADER_H
#define SHADER_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);
    void bind();
    void unbind();
    GLint getUniformLocation(const char* name);
	void setUniform1i(const char* name, int v);
	void setUniform1iv(const char* name, int length, int* v);
	void setUniform1f(const char* name, float v);
	void setUniform4f(const char* name, float v0, float v1, float v2, float v3);
    void SetUniformMatrix4fv(const char* name, const glm::mat4& matrix);

private:
    uint32_t m_ProgramId;
    std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

#endif // SHADER_H
