#ifndef MESH_H
#define MESH_H

#include <vector>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

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