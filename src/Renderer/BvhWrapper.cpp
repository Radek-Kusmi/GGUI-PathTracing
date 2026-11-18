#include "BvhWrapper.h"
#include "../Scene/Mesh.h"
#include <numeric>

void BvhWrapper::build(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
    uint32_t numTriangles = static_cast<uint32_t>(indices.size() / 3);
    if (numTriangles == 0) return;

    nodes.clear();
    nodes.reserve(numTriangles * 2);
    triIndices.resize(numTriangles);
    std::iota(triIndices.begin(), triIndices.end(), 0);

    // Prekalkulacja środków ciężkości wszystkich trójkątów
    std::vector<glm::vec3> centroids(numTriangles);
    for (uint32_t i = 0; i < numTriangles; ++i) {
        const glm::vec3& p0 = vertices[indices[i * 3 + 0]].position;
        const glm::vec3& p1 = vertices[indices[i * 3 + 1]].position;
        const glm::vec3& p2 = vertices[indices[i * 3 + 2]].position;
        centroids[i] = (p0 + p1 + p2) * (1.0f / 3.0f);
    }

    BVHNode root{};
    root.leftFirst = 0;
    root.triCount = numTriangles;
    nodes.push_back(root);

    subdivide(0, centroids, vertices, indices);
}

void BvhWrapper::subdivide(
    uint32_t nodeIdx,
    const std::vector<glm::vec3>& centroids,
    const std::vector<Vertex>& vertices,
    const std::vector<unsigned int>& indices
) {
    BVHNode& node = nodes[nodeIdx];

    // Wyznaczamy AABB obejmujące trójkąty w bieżącym węźle
    glm::vec3 bMin(1e30f);
    glm::vec3 bMax(-1e30f);
    for (uint32_t i = 0; i < node.triCount; ++i) {
        uint32_t triIdx = triIndices[node.leftFirst + i];
        const glm::vec3& p0 = vertices[indices[triIdx * 3 + 0]].position;
        const glm::vec3& p1 = vertices[indices[triIdx * 3 + 1]].position;
        const glm::vec3& p2 = vertices[indices[triIdx * 3 + 2]].position;

        bMin = glm::min(bMin, glm::min(p0, glm::min(p1, p2)));
        bMax = glm::max(bMax, glm::max(p0, glm::max(p1, p2)));
    }
    node.aabbMin = bMin - glm::vec3(1e-4f);
    node.aabbMax = bMax + glm::vec3(1e-4f);

    // Warunek stopu (liść: maks. 4 trójkąty w pudełku)
    if (node.triCount <= 4) {
        return;
    }

    // Wybór najdłuższej osi pudła (0 = X, 1 = Y, 2 = Z)
    glm::vec3 extent = node.aabbMax - node.aabbMin;
    int axis = 0;
    if (extent.y > extent.x) axis = 1;
    if (extent.z > extent[axis]) axis = 2;

    float splitPos = node.aabbMin[axis] + extent[axis] * 0.5f;

    // Partycjonowanie trójkątów in-place
    uint32_t i = node.leftFirst;
    uint32_t j = i + node.triCount - 1;
    while (i <= j) {
        if (centroids[triIndices[i]][axis] < splitPos) {
            i++;
        }
        else {
            std::swap(triIndices[i], triIndices[j]);
            if (j == 0) break;
            j--;
        }
    }

    uint32_t leftCount = i - node.leftFirst;
    if (leftCount == 0 || leftCount == node.triCount) {
        leftCount = node.triCount / 2;
        uint32_t mid = node.leftFirst + leftCount;
        std::nth_element(
            triIndices.begin() + node.leftFirst,
            triIndices.begin() + mid,
            triIndices.begin() + node.leftFirst + node.triCount,
            [&](uint32_t a, uint32_t b) { return centroids[a][axis] < centroids[b][axis]; }
        );
    }

    uint32_t leftChildIdx = static_cast<uint32_t>(nodes.size());
    uint32_t rightChildIdx = leftChildIdx + 1;

    BVHNode leftNode{};
    leftNode.leftFirst = node.leftFirst;
    leftNode.triCount = leftCount;

    BVHNode rightNode{};
    rightNode.leftFirst = node.leftFirst + leftCount;
    rightNode.triCount = node.triCount - leftCount;

    nodes[nodeIdx].leftFirst = leftChildIdx;
    nodes[nodeIdx].triCount = 0;

    nodes.push_back(leftNode);
    nodes.push_back(rightNode);

    subdivide(leftChildIdx, centroids, vertices, indices);
    subdivide(rightChildIdx, centroids, vertices, indices);
}