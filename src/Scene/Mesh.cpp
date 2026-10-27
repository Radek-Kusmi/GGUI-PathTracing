#include "mesh.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>

Mesh::Mesh() : VAO(0), VBO(0), EBO(0), isSetup(false) {}

Mesh::~Mesh() {
    if (isSetup) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
    }
}

// Funkcja pomocnicza do ładowania kolorów Kd z pliku .mtl
bool Mesh::loadMTL(const std::string& mtlPath, std::unordered_map<std::string, glm::vec3>& materialColors) {
    std::ifstream file(mtlPath);
    if (!file.is_open()) {
        std::cerr << "Ostrzezenie: Nie mozna otworzyc pliku MTL: " << mtlPath << std::endl;
        return false;
    }

    std::string line;
    std::string currentMaterial = "";

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "newmtl") {
            ss >> currentMaterial;
        }
        else if (prefix == "Kd" && !currentMaterial.empty()) {
            glm::vec3 color;
            ss >> color.r >> color.g >> color.b;
            materialColors[currentMaterial] = color;
        }
    }

    file.close();
    return true;
}

bool Mesh::loadFromOBJ(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Blad: Nie mozna otworzyc pliku " << filepath << std::endl;
        return false;
    }

    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_texCoords;
    std::vector<glm::vec3> temp_normals;

    std::unordered_map<std::string, glm::vec3> materialColors;
    glm::vec3 currentKdColor = glm::vec3(0.8f, 0.8f, 0.8f); // Domyslny jasnoszary kolor

    // Wyciąganie ścieżki do katalogu pliku OBJ (aby poprawnie zlokalizować plik MTL)
    std::string directory = "";
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        directory = filepath.substr(0, lastSlash + 1);
    }

    vertices.clear();
    indices.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        // Ładowanie biblioteki materiałów (.mtl)
        if (prefix == "mtllib") {
            std::string mtlFilename;
            ss >> mtlFilename;
            loadMTL(directory + mtlFilename, materialColors);
        }
        // Wybór aktywnego materiału dla kolejnych ścian
        else if (prefix == "usemtl") {
            std::string mtlName;
            ss >> mtlName;
            if (materialColors.find(mtlName) != materialColors.end()) {
                currentKdColor = materialColors[mtlName];
            }
        }
        else if (prefix == "v") {
            glm::vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_positions.push_back(pos);
        }
        else if (prefix == "vt") {
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_texCoords.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 norm;
            ss >> norm.x >> norm.y >> norm.z;
            temp_normals.push_back(norm);
        }
        else if (prefix == "f") {
            std::vector<std::string> faceVertices;
            std::string vertexStr;
            while (ss >> vertexStr) {
                faceVertices.push_back(vertexStr);
            }

            for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
                std::string cornerIndices[3] = { faceVertices[0], faceVertices[i], faceVertices[i + 1] };

                for (int j = 0; j < 3; ++j) {
                    std::stringstream vss(cornerIndices[j]);
                    std::string vIdxStr, vtIdxStr, vnIdxStr;

                    std::getline(vss, vIdxStr, '/');
                    std::getline(vss, vtIdxStr, '/');
                    std::getline(vss, vnIdxStr, '/');

                    Vertex vertex{};

                    int vIdx = vIdxStr.empty() ? 0 : std::stoi(vIdxStr);
                    int vtIdx = vtIdxStr.empty() ? 0 : std::stoi(vtIdxStr);
                    int vnIdx = vnIdxStr.empty() ? 0 : std::stoi(vnIdxStr);

                    if (vIdx > 0 && vIdx <= static_cast<int>(temp_positions.size())) {
                        vertex.position = temp_positions[vIdx - 1];
                    }

                    if (vtIdx > 0 && vtIdx <= static_cast<int>(temp_texCoords.size())) {
                        vertex.texCoords = temp_texCoords[vtIdx - 1];
                    }
                    else {
                        vertex.texCoords = glm::vec2(0.0f, 0.0f);
                    }

                    if (vnIdx > 0 && vnIdx <= static_cast<int>(temp_normals.size())) {
                        vertex.normal = temp_normals[vnIdx - 1];
                    }
                    else {
                        vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }

                    // Przypisanie koloru materiału do wierzchołka
                    vertex.color = currentKdColor;

                    vertices.push_back(vertex);
                    indices.push_back(static_cast<unsigned int>(indices.size()));
                }
            }
        }
    }

    file.close();
    setupMesh();
    return true;
}

void Mesh::setupMesh() {
    if (vertices.empty()) return;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Attribute 0: Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

    // Attribute 1: TexCoords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoords));

    // Attribute 2: Normals
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));

    // Attribute 3: Color (NOWOŚĆ)
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));

    glBindVertexArray(0);
    isSetup = true;
}

void Mesh::draw() const {
    if (!isSetup) return;
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}