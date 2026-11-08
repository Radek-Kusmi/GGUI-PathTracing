#include "Scene.h"
#include <iostream>

bool Scene::addMesh(const std::string& filepath) {
    Mesh mesh;
    if (mesh.loadFromOBJ(filepath)) {
        meshes.push_back(mesh);
        return true;
    }
    return false;
}

void Scene::addMesh(const Mesh& mesh) {
    meshes.push_back(mesh);
}

void Scene::addLight(const glm::vec3& position, const glm::vec3& color, float intensity) {
    lights.push_back({ position, color, intensity });
}

void Scene::printInfo() const {
    std::cout << "\n================ PODSUMOWANIE SCENY ================" << std::endl;
    std::cout << "Liczba wczytanych obiektów w scenie: " << meshes.size() << std::endl;

    size_t totalTriangles = 0;
    for (size_t i = 0; i < meshes.size(); ++i) {
        size_t triCount = meshes[i].indices.size() / 3;
        totalTriangles += triCount;
        std::cout << "  - Obiekt [" << i << "]: " << meshes[i].vertices.size()
            << " wierzchołków, " << triCount << " trójkątów" << std::endl;
    }

    std::cout << "Łączna liczba trójkątów: " << totalTriangles << std::endl;
    std::cout << "Liczba źródeł światła: " << lights.size() << std::endl;
    std::cout << "==================================================\n" << std::endl;
}