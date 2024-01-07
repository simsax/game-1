#include "shader.h"
#include "utils.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath) {
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
        fprintf(stderr, "Vertex shader compilation failed with error: %s\n", info);
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
        fprintf(stderr, "Fragment shader compilation failed with error: %s\n", info);
        exit(EXIT_FAILURE);
    }

    // link shaders to shader program
    m_ProgramId = glCreateProgram();
    glAttachShader(m_ProgramId, vertexShader);
    glAttachShader(m_ProgramId, fragmentShader);
    glLinkProgram(m_ProgramId);

    glGetProgramiv(m_ProgramId, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(m_ProgramId, 512, NULL, info);
        fprintf(stderr, "Shader linking failed with error: %s\n", info);
        exit(EXIT_FAILURE);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::bind() {
    glUseProgram(m_ProgramId);
}

void Shader::unbind() {
    glUseProgram(0);
}

GLint Shader::getUniformLocation(const char* name)
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];

	GLint location = glGetUniformLocation(m_ProgramId, name);
	if (location == -1) {
		fprintf(stderr, "Uniform '%s' location not found.\n", name);
        exit(EXIT_FAILURE);
    }
	m_UniformLocationCache[name] = location;
	return location;
}

void Shader::setUniform1i(const char* name, int v) {
    glUniform1i(getUniformLocation(name), v);
}

void Shader::setUniform1iv(const char* name, int length, int* v) {
    glUniform1iv(getUniformLocation(name), length, v);
}

void Shader::setUniform1f(const char* name, float v) {
    glUniform1f(getUniformLocation(name), v);
}

void Shader::setUniform4f(const char* name, float v0, float v1, float v2, float v3) {
    glUniform4f(getUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMatrix4fv(const char* name, const glm::mat4& matrix) {
    glUniformMatrix4fv(getUniformLocation(name), 1, GL_FALSE, glm::value_ptr(matrix));
}
