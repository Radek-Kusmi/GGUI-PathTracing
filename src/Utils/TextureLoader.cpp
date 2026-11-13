#include "TextureLoader.h"
#include <GL/glew.h>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

unsigned int loadTexture(const std::string& filepath) {
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &nrComponents, 0);

    if (data) {
        GLenum format = GL_RGB;
        if (nrComponents == 1) format = GL_RED;
        else if (nrComponents == 3) format = GL_RGB;
        else if (nrComponents == 4) format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);

        // KLUCZOWA POPRAWKA: Pozwala ładować obrazki o dowolnych wymiarach (np. nieparzystych)
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Ustawienia brzegów i filtrowania
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
        std::cout << "Pomyslnie wczytano teksture (" << width << "x" << height << ", kanaly: " << nrComponents << "): " << filepath << std::endl;
    }
    else {
        std::cerr << "BLAD: Nie można otworzyc pliku tekstury: " << filepath << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

CpuTexture loadCpuTexture(const std::string& filepath) {
    CpuTexture tex;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* raw = stbi_load(filepath.c_str(), &tex.width, &tex.height, &tex.channels, 0);
    if (raw) {
        size_t totalBytes = static_cast<size_t>(tex.width) * tex.height * tex.channels;
        tex.data.assign(raw, raw + totalBytes);
        stbi_image_free(raw);
    }
    else {
        std::cerr << "BLAD (CPU Texture): Nie mozna wczytac " << filepath << std::endl;
    }
    return tex;
}