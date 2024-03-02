#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <glad/glad.h>
#include "Config.h"

#define ABS_PATH(x) (PROJECT_SOURCE_DIR x)

std::string readFile(const char* path);
void APIENTRY glDebugOutput(GLenum source, GLenum type, unsigned int id, GLenum severity,
                            GLsizei length, const char *message, const void *userParam);

#endif //  UTILS_H
