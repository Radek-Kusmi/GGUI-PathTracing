#ifndef BVH_WRAPPER_H
#define BVH_WRAPPER_H

#include <vector>
#include <cstdint>
#include <algorithm>
#include <glm/glm.hpp>
#include "../Core/Ray.h"

// Forward declaration, aby uniknąć cyklicznego dołączania nagłówków z Mesh.h
struct Vertex;

struct BVHNode {
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;
    uint32_t leftFirst; // Jeśli triCount > 0: indeks pierwszego trójkąta; w przeciwnym razie: indeks lewego dziecka
    uint32_t triCount;  // 0 = węzeł wewnętrzny, > 0 = liść (zawiera trójkąty)

    bool isLeaf() const { return triCount > 0; }
};

class BvhWrapper {
public:
    std::vector<BVHNode> nodes;
    std::vector<uint32_t> triIndices;

    BvhWrapper() = default;

    // Buduje drzewo BVH dla podanej siatki wierzchołków i indeksów
    void build(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);

    bool isBuilt() const { return !nodes.empty(); }

    // Błyskawiczny test przecięcia promienia z pudełkiem AABB
    static inline bool intersectAABB(
        const Ray& ray,
        const glm::vec3& bMin,
        const glm::vec3& bMax,
        float tMin,
        float tMax
    ) {
        float tx1 = (bMin.x - ray.origin.x) * ray.invDir.x;
        float tx2 = (bMax.x - ray.origin.x) * ray.invDir.x;
        float tmin = std::min(tx1, tx2);
        float tmax = std::max(tx1, tx2);

        float ty1 = (bMin.y - ray.origin.y) * ray.invDir.y;
        float ty2 = (bMax.y - ray.origin.y) * ray.invDir.y;
        tmin = std::max(tmin, std::min(ty1, ty2));
        tmax = std::min(tmax, std::max(ty1, ty2));

        float tz1 = (bMin.z - ray.origin.z) * ray.invDir.z;
        float tz2 = (bMax.z - ray.origin.z) * ray.invDir.z;
        tmin = std::max(tmin, std::min(tz1, tz2));
        tmax = std::min(tmax, std::max(tz1, tz2));

        return tmax >= std::max(tmin, tMin) && tmin < tMax;
    }

private:
    void subdivide(
        uint32_t nodeIdx,
        const std::vector<glm::vec3>& centroids,
        const std::vector<Vertex>& vertices,
        const std::vector<unsigned int>& indices
    );
};

#endif // BVH_WRAPPER_H