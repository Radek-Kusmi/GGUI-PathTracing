#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <vector>

// Dołączenie GLM
#include <glm/glm.hpp>

// Wdrożenie stb_image_write w jednym pliku .cpp
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

int main() {
    const int width = 800;
    const int height = 600;
    const int channels = 3; // RGB

    std::vector<unsigned char> pixels(width * height * channels);

    // Wygenerujmy prosty gradient jako obrazek testowy
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int index = (y * width + x) * channels;

            // Użycie GLM do obliczenia koloru na podstawie pozycji
            glm::vec3 color(
                static_cast<float>(x) / width,  // Czerwony (R)
                static_cast<float>(y) / height, // Zielony (G)
                0.2f                            // Niebieski (B)
            );

            pixels[index + 0] = static_cast<unsigned char>(color.r * 255.0f);
            pixels[index + 1] = static_cast<unsigned char>(color.g * 255.0f);
            pixels[index + 2] = static_cast<unsigned char>(color.b * 255.0f);
        }
    }

    // Zapis do PNG
    const char* filename = "test_output.png";
    if (stbi_write_png(filename, width, height, channels, pixels.data(), width * channels)) {
        std::cout << "[SUCCESS] Obrazek testowy zapisany jako " << filename << std::endl;
    }
    else {
        std::cerr << "[ERROR] Nie udalo sie zapisac obrazu!" << std::endl;
    }

    return 0;
}