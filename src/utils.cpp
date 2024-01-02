#include "utils.h"
#include <fstream>
#include <iostream>
#include <sstream>

std::string readFile(const char* path) {
    std::ifstream inputFile{ path };
    std::stringstream fileString;

    if (!inputFile) {
        std::cerr << "Failed to open file: " << path << "\n";
        exit(EXIT_FAILURE);
    }

    fileString << inputFile.rdbuf();

    return fileString.str();
}
