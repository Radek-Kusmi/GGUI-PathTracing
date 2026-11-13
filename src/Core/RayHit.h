#ifndef RAYHIT_H
#define RAYHIT_H

#include <glm/glm.hpp>
#include <limits>
#include "ray.h"
#include "../Scene/Mesh.h"

struct RayHit {
    bool hit = false;
    float t = std::numeric_limits<float>::max(); // Odległość od początku promienia
    glm::vec3 point = glm::vec3(0.0f);           // Punkt przecięcia w przestrzeni świata
    glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 color = glm::vec3(1.0f);           // Kolor bazowy (albedo) w punkcie trafienia
    glm::vec2 texCoords = glm::vec2(0.0f);       // Interpolowane UV
    bool frontFace = true;                       // Czy promień uderzył od zewnątrz (ważne przy refrakcji lodu!)
    bool isIce = false;                          // Flaga pozwalająca odróżnić lód od innych obiektów

    // Ustawia wektor normalny zawsze przeciwnie do kierunku promienia
    void setFaceNormal(const Ray& ray, const glm::vec3& outwardNormal) {
        frontFace = glm::dot(ray.direction, outwardNormal) < 0.0f;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }
};

// Przecięcie promienia z pojedynczym trójkątem (Algorytm Möller–Trumbore)
inline bool intersectTriangle(
    const Ray& ray,
    const Vertex& v0,
    const Vertex& v1,
    const Vertex& v2,
    const glm::mat4& modelMatrix,
    const glm::mat3& normalMatrix,
    float tMin,
    float tMax,
    RayHit& outHit
) {
    const float EPSILON = 1e-6f;

    // Transformacja wierzchołków z przestrzeni lokalnej modelu do przestrzeni świata
    glm::vec3 p0 = glm::vec3(modelMatrix * glm::vec4(v0.position, 1.0f));
    glm::vec3 p1 = glm::vec3(modelMatrix * glm::vec4(v1.position, 1.0f));
    glm::vec3 p2 = glm::vec3(modelMatrix * glm::vec4(v2.position, 1.0f));

    glm::vec3 edge1 = p1 - p0;
    glm::vec3 edge2 = p2 - p0;

    glm::vec3 h = glm::cross(ray.direction, edge2);
    float a = glm::dot(edge1, h);

    // Jeśli a jest bliskie 0, promień biegnie równolegle do trójkąta
    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    glm::vec3 s = ray.origin - p0;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(ray.direction, q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    // Wyliczenie odległości t wzdłuż promienia
    float t = f * glm::dot(edge2, q);

    if (t < tMin || t > tMax) {
        return false;
    }

    // Zapisanie danych trafienia
    outHit.hit = true;
    outHit.t = t;
    outHit.point = ray.at(t);

    // Współrzędne barycentryczne do płynnej interpolacji atrybutów wierzchołków
    float w = 1.0f - u - v;

    glm::vec3 localNormal = glm::normalize(w * v0.normal + u * v1.normal + v * v2.normal);
    glm::vec3 worldNormal = glm::normalize(normalMatrix * localNormal);
    outHit.setFaceNormal(ray, worldNormal);

    outHit.color = w * v0.color + u * v1.color + v * v2.color;
    outHit.texCoords = w * v0.texCoords + u * v1.texCoords + v * v2.texCoords;

    return true;
}

// Sprawdza przecięcie promienia ze wszystkimi trójkątami całego obiektu Mesh
inline bool intersectMesh(
    const Ray& ray,
    const Mesh& mesh,
    const glm::mat4& modelMatrix,
    float tMin,
    float tMax,
    RayHit& closestHit,
    bool isIce = false
) {
    bool hitAnything = false;
    float currentClosest = tMax;

    // Macierz normalnych liczymy raz dla całego obiektu, a nie dla każdego trójkąta osobno (oszczędność CPU)
    glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));

    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const Vertex& v0 = mesh.vertices[mesh.indices[i]];
        const Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
        const Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

        RayHit tempHit;
        if (intersectTriangle(ray, v0, v1, v2, modelMatrix, normalMatrix, tMin, currentClosest, tempHit)) {
            hitAnything = true;
            currentClosest = tempHit.t;
            closestHit = tempHit;
            closestHit.isIce = isIce;
        }
    }

    return hitAnything;
}

#endif // RAYHIT_H