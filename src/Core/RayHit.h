#ifndef RAYHIT_H
#define RAYHIT_H

#include <glm/glm.hpp>
#include <limits>
#include <algorithm>
#include "ray.h"
#include "../Scene/Mesh.h"

struct RayHit {
    bool hit = false;
    float t = std::numeric_limits<float>::max(); // Odległość od początku promienia w przestrzeni świata
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

// Błyskawiczny test przecięcia promienia z prostopadłościanem AABB (Slab Test z użyciem invDir)
inline bool intersectAABB(
    const Ray& localRay,
    const glm::vec3& boxMin,
    const glm::vec3& boxMax,
    float tMin,
    float tMax
) {
    for (int i = 0; i < 3; ++i) {
        float t0 = (boxMin[i] - localRay.origin[i]) * localRay.invDir[i];
        float t1 = (boxMax[i] - localRay.origin[i]) * localRay.invDir[i];
        if (localRay.invDir[i] < 0.0f) {
            std::swap(t0, t1);
        }
        tMin = t0 > tMin ? t0 : tMin;
        tMax = t1 < tMax ? t1 : tMax;
        if (tMax <= tMin) {
            return false;
        }
    }
    return true;
}

// Przecięcie promienia z pojedynczym trójkątem w przestrzeni lokalnej (bez mnożenia macierzy!)
inline bool intersectTriangleLocal(
    const Ray& localRay,
    const Vertex& v0,
    const Vertex& v1,
    const Vertex& v2,
    float tMin,
    float tMax,
    float& outT,
    float& outU,
    float& outV
) {
    const float EPSILON = 1e-6f;

    glm::vec3 edge1 = v1.position - v0.position;
    glm::vec3 edge2 = v2.position - v0.position;

    glm::vec3 h = glm::cross(localRay.direction, edge2);
    float a = glm::dot(edge1, h);

    if (a > -EPSILON && a < EPSILON) {
        return false;
    }

    float f = 1.0f / a;
    glm::vec3 s = localRay.origin - v0.position;
    float u = f * glm::dot(s, h);

    if (u < 0.0f || u > 1.0f) {
        return false;
    }

    glm::vec3 q = glm::cross(s, edge1);
    float v = f * glm::dot(localRay.direction, q);

    if (v < 0.0f || u + v > 1.0f) {
        return false;
    }

    float t = f * glm::dot(edge2, q);
    if (t < tMin || t > tMax) {
        return false;
    }

    outT = t;
    outU = u;
    outV = v;
    return true;
}

// Sprawdza przecięcie promienia ze wszystkimi trójkątami obiektu Mesh w przestrzeni lokalnej
// Przyjmuje wstępnie wyliczone invModel oraz normalMatrix (zamiast odwracać macierze dla każdego promienia)
inline bool intersectMesh(
    const Ray& worldRay,
    const Mesh& mesh,
    const glm::mat4& invModel,
    const glm::mat3& normalMatrix,
    float tMin,
    float tMax,
    RayHit& closestHit,
    bool isIce = false
) {
    if (mesh.indices.empty() || mesh.vertices.empty()) {
        return false;
    }

    // 1. Przenosimy TYLKO promień z przestrzeni świata do przestrzeni lokalnej obiektu
    glm::vec3 localOrigin = glm::vec3(invModel * glm::vec4(worldRay.origin, 1.0f));
    glm::vec3 localDir = glm::vec3(invModel * glm::vec4(worldRay.direction, 0.0f));

    // normalizeDir = false: zachowujemy skalę obiektu, dzięki czemu parametr 't' odpowiada dystansowi w świecie!
    Ray localRay(localOrigin, localDir, false);

    // 2. Błyskawiczny test z pre-kalkulowanym lokalnym AABB obiektu
    if (mesh.aabbMin != mesh.aabbMax) {
        if (!intersectAABB(localRay, mesh.aabbMin, mesh.aabbMax, tMin, tMax)) {
            return false;
        }
    }

    // 3. Testujemy surowe wierzchołki siatki bez żadnych operacji macierzowych
    bool hitAnything = false;
    float currentClosest = tMax;
    float bestU = 0.0f, bestV = 0.0f;
    size_t bestIndex = 0;

    for (size_t i = 0; i + 2 < mesh.indices.size(); i += 3) {
        const Vertex& v0 = mesh.vertices[mesh.indices[i]];
        const Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
        const Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];

        float t, u, v;
        if (intersectTriangleLocal(localRay, v0, v1, v2, tMin, currentClosest, t, u, v)) {
            hitAnything = true;
            currentClosest = t;
            bestU = u;
            bestV = v;
            bestIndex = i;
        }
    }

    // 4. Interpolację normalnych, koloru i UV wykonujemy TYLKO RAZ dla najbliższego trafionego trójkąta!
    if (hitAnything) {
        const Vertex& v0 = mesh.vertices[mesh.indices[bestIndex]];
        const Vertex& v1 = mesh.vertices[mesh.indices[bestIndex + 1]];
        const Vertex& v2 = mesh.vertices[mesh.indices[bestIndex + 2]];

        float w = 1.0f - bestU - bestV;

        closestHit.hit = true;
        closestHit.t = currentClosest;
        closestHit.point = worldRay.at(currentClosest);
        closestHit.isIce = isIce;

        glm::vec3 localNormal = glm::normalize(w * v0.normal + bestU * v1.normal + bestV * v2.normal);
        glm::vec3 worldNormal = glm::normalize(normalMatrix * localNormal);
        closestHit.setFaceNormal(worldRay, worldNormal);

        closestHit.color = w * v0.color + bestU * v1.color + bestV * v2.color;
        closestHit.texCoords = w * v0.texCoords + bestU * v1.texCoords + bestV * v2.texCoords;
    }

    return hitAnything;
}

#endif // RAYHIT_H