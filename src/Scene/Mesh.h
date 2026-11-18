#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "../Renderer/BvhWrapper.h"

struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoords;
    glm::vec3 normal;
    glm::vec3 color;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    BvhWrapper bvh;

    // Pre-kalkulowane lokalne pudełko otaczające AABB dla szybkich testów Ray Tracingu
    glm::vec3 aabbMin = glm::vec3(0.0f);
    glm::vec3 aabbMax = glm::vec3(0.0f);

    Mesh();
    ~Mesh();

    bool loadFromOBJ(const std::string& filepath);
    void setupMesh();
    void draw() const;

private:
    GLuint VAO, VBO, EBO;
    bool isSetup;

    bool loadMTL(const std::string& mtlPath, std::unordered_map<std::string, glm::vec3>& materialColors);
};

#endif // MESH_H