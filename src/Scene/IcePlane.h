#ifndef ICE_PLANE_H
#define ICE_PLANE_H

#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include "Scene/Mesh.h"

// Funkcja pomocnicza do generowania prostego szumu pseudo-losowego (0.0 - 1.0)
inline float pseudoRandom(float x, float z) {
    float dotProduct = x * 12.9898f + z * 78.233f;
    return std::fmod(std::sin(dotProduct) * 43758.5453f, 1.0f);
}

// Generuje płaską taflę lodu z losowymi, drobnymi pocharataniami
inline Mesh generateIcePlane(float width, float depth, int gridRes, float roughness) {
    Mesh iceMesh;
    float halfW = width * 0.5f;
    float halfD = depth * 0.5f;
    float dx = width / gridRes;
    float dz = depth / gridRes;

    // 1. Generowanie wierzchołków
    for (int i = 0; i <= gridRes; ++i) {
        for (int j = 0; j <= gridRes; ++j) {
            Vertex v;
            float x = -halfW + j * dx;
            float z = -halfD + i * dz;

            // Łączymy szum z delikatnymi nieliniowymi częstotliwościami dla przełamania symetrii
            float microNoise = (std::sin(x * 3.5f + z * 2.1f) * std::cos(x * 1.7f - z * 4.3f)) * 0.4f;
            float sharpCracks = (std::sin(x * 12.0f) * std::cos(z * 15.0f)) * 0.15f;

            // Wysokość Y: podstawa jest niemal idealnie płaska, rzeźbiona drobnym szumem
            float y = (microNoise + sharpCracks) * roughness * 0.2f;

            v.position = glm::vec3(x, y, z);

            // Mapowanie UV: rozciągnięcie całej tekstury na całą taflę lodu (0.0 do 1.0)
            float u = (float)j / (float)gridRes;
            float vCoord = (float)i / (float)gridRes;
            v.texCoords = glm::vec2(u, vCoord);

            // Kolor neutralny (biel), aby w pełni uwydatnić teksturę z pliku
            v.color = glm::vec3(1.0f, 1.0f, 1.0f);

            v.normal = glm::vec3(0.0f, 1.0f, 0.0f);

            iceMesh.vertices.push_back(v);
        }
    }

    // 2. Generowanie siatki trójkątów (indeksów)
    for (int i = 0; i < gridRes; ++i) {
        for (int j = 0; j < gridRes; ++j) {
            int row1 = i * (gridRes + 1);
            int row2 = (i + 1) * (gridRes + 1);

            int idx0 = row1 + j;
            int idx1 = row2 + j;
            int idx2 = row1 + j + 1;
            int idx3 = row2 + j + 1;

            iceMesh.indices.push_back(idx0);
            iceMesh.indices.push_back(idx1);
            iceMesh.indices.push_back(idx2);

            iceMesh.indices.push_back(idx2);
            iceMesh.indices.push_back(idx1);
            iceMesh.indices.push_back(idx3);
        }
    }

    // 3. Rekonstrukcja wektorów normalnych pod cieniowanie i path tracing
    std::vector<glm::vec3> tempNormals(iceMesh.vertices.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < iceMesh.indices.size(); i += 3) {
        unsigned int i0 = iceMesh.indices[i];
        unsigned int i1 = iceMesh.indices[i + 1];
        unsigned int i2 = iceMesh.indices[i + 2];

        glm::vec3 p0 = iceMesh.vertices[i0].position;
        glm::vec3 p1 = iceMesh.vertices[i1].position;
        glm::vec3 p2 = iceMesh.vertices[i2].position;

        glm::vec3 faceNormal = glm::cross(p1 - p0, p2 - p0);

        tempNormals[i0] += faceNormal;
        tempNormals[i1] += faceNormal;
        tempNormals[i2] += faceNormal;
    }

    for (size_t i = 0; i < iceMesh.vertices.size(); ++i) {
        iceMesh.vertices[i].normal = glm::normalize(tempNormals[i]);
    }

    return iceMesh;
}

#endif // ICE_PLANE_H