#ifndef SHADER_LOADER_H
#define SHADER_LOADER_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

inline std::string loadShaderSource(const char* filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Błąd wczytywania pliku shadera: " << filePath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

#endif
