#include "shader.h"
#include "utils.h"
#include "logger.h"

Shader::Shader() : 
    m_ProgramId(0),
    m_UniformLocationCache({})
{}

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath) :
    m_ProgramId(0),
    m_UniformLocationCache({})
{
    // compile vertex shader
    std::string vertexShaderSource = readFile(vertexShaderPath);
    uint32_t vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vs = vertexShaderSource.c_str();
    glShaderSource(vertexShader, 1, &vs, NULL);
    glCompileShader(vertexShader);
    int success;
    char info[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, info);
        LOG_ERROR("Vertex shader compilation failed with error: {}", info);
        exit(EXIT_FAILURE);
    }

    // compile fragment shader
    std::string fragmentShaderSource = readFile(fragmentShaderPath);
    uint32_t fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fs = fragmentShaderSource.c_str();
    glShaderSource(fragmentShader, 1, &fs, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, info);
        LOG_ERROR("Fragment shader compilation failed with error: {}", info);
        exit(EXIT_FAILURE);
    }

    // link shaders to shader program
    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, vertexShader);
    glAttachShader(m_ProgramId, fragmentShader);
    glLinkProgram(m_ProgramId);

    glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_ProgramId, 512, NULL, info);
        LOG_ERROR("Shader linking failed with error: {}", info);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::Bind() {
    glUseProgram(m_ProgramId);
}

void Shader::Unbind() {
    glUseProgram(0);
}

GLint Shader::GetUniformLocation(const char* name)
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];

	GLint location = glGetUniformLocation(m_ProgramId, name);
	if (location == -1) {
		LOG_ERROR("Uniform '{}' location not found.", name);
        exit(EXIT_FAILURE);
    }
	m_UniformLocationCache[name] = location;
	return location;
}

void Shader::SetUniform1i(const char* name, int v) {
    glUniform1i(GetUniformLocation(name), v);
}

void Shader::SetUniform1iv(const char* name, int length, int* v) {
    glUniform1iv(GetUniformLocation(name), length, v);
}

void Shader::SetUniform1f(const char* name, float v) {
    glUniform1f(GetUniformLocation(name), v);
}


void Shader::SetUniform2f(const char* name, float v0, float v1) {
    glUniform2f(GetUniformLocation(name), v0, v1);
}

void Shader::SetUniform3f(const char* name, float v0, float v1, float v2) {
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}


void Shader::SetUniform4f(const char* name, float v0, float v1, float v2, float v3) {
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMatrix4fv(const char* name, const glm::mat4& matrix) {
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}
