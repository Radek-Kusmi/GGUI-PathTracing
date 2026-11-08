#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

class Scene {
public:
    std::vector<Mesh> meshes;
    std::vector<PointLight> lights;

    // 1. Dodawanie siatki z pliku OBJ (np. Latarnia)
    bool addMesh(const std::string& filepath);

    // 2. Dodawanie gotowej siatki z pamięci RAM (np. Proceduralny Lód)
    void addMesh(const Mesh& mesh);

    // Dodawanie źródła światła
    void addLight(const glm::vec3& position, const glm::vec3& color, float intensity);

    // Wypisywanie statystyk sceny
    void printInfo() const;
};