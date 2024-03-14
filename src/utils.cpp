#include "utils.h"
#include <fstream>
#include <sstream>
#include "logger.h"

std::string readFile(const char* path) {
    std::ifstream inputFile{ path };
    std::stringstream fileString;

    if (!inputFile) {
        LOG_ERROR("Failed to open file: {}", path);
        exit(EXIT_FAILURE);
    }

    fileString << inputFile.rdbuf();

    return fileString.str();
}

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    LOG_WARN("---------------");
    LOG_WARN("Debug message ({}): {}", id , message);

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             LOG_WARN("Source: API"); break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   LOG_WARN("Source: Window System"); break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: LOG_WARN("Source: Shader Compiler"); break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     LOG_WARN("Source: Third Party"); break;
        case GL_DEBUG_SOURCE_APPLICATION:     LOG_WARN("Source: Application"); break;
        case GL_DEBUG_SOURCE_OTHER:           LOG_WARN("Source: Other"); break;
    }

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               LOG_WARN("Type: Error"); break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: LOG_WARN("Type: Deprecated Behaviour"); break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  LOG_WARN("Type: Undefined Behaviour"); break; 
        case GL_DEBUG_TYPE_PORTABILITY:         LOG_WARN("Type: Portability"); break;
        case GL_DEBUG_TYPE_PERFORMANCE:         LOG_WARN("Type: Performance"); break;
        case GL_DEBUG_TYPE_MARKER:              LOG_WARN("Type: Marker"); break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          LOG_WARN("Type: Push Group"); break;
        case GL_DEBUG_TYPE_POP_GROUP:           LOG_WARN("Type: Pop Group"); break;
        case GL_DEBUG_TYPE_OTHER:               LOG_WARN("Type: Other"); break;
    }
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         LOG_WARN("Severity: high"); break;
        case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("Severity: medium"); break;
        case GL_DEBUG_SEVERITY_LOW:          LOG_WARN("Severity: low"); break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_WARN("Severity: notification"); break;
    }

    std::cout << "\n";
}

