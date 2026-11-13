#ifndef TEXTURE_LOADER_H
#define TEXTURE_LOADER_H

#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>

// Deklaracja funkcji ładującej teksturę do OpenGL
unsigned int loadTexture(const std::string& filepath);

// Struktura tekstury w pamięci RAM dla CPU Path Tracera
struct CpuTexture {
    int width = 0;
    int height = 0;
    int channels = 0;
    std::vector<unsigned char> data;

    // Pobiera kolor RGB (0.0 - 1.0) dla współrzędnych UV (0.0 - 1.0)
    glm::vec3 sample(float u, float v) const {
        if (data.empty() || width <= 0 || height <= 0) {
            return glm::vec3(0.8f, 0.92f, 1.0f); // Domyślny błękitny kolor awaryjny
        }

        // Ograniczenie UV do zakresu [0.0, 1.0] (odpowiednik GL_CLAMP_TO_EDGE)
        u = std::clamp(u, 0.0f, 0.9999f);
        v = std::clamp(v, 0.0f, 0.9999f);

        int x = static_cast<int>(u * width);
        int y = static_cast<int>(v * height);

        int index = (y * width + x) * channels;
        float r = data[index] / 255.0f;
        float g = (channels > 1) ? (data[index + 1] / 255.0f) : r;
        float b = (channels > 2) ? (data[index + 2] / 255.0f) : r;

        return glm::vec3(r, g, b);
    }
};

// Funkcja wczytująca obrazek do RAM
CpuTexture loadCpuTexture(const std::string& filepath);

#endif // TEXTURE_LOADER_H