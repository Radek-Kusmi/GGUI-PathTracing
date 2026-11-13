#ifndef RAY_H
#define RAY_H

#include <glm/glm.hpp>

class Ray {
public:
    glm::vec3 origin;
    glm::vec3 direction;

    Ray() : origin(0.0f), direction(0.0f, 0.0f, -1.0f) {}
    Ray(const glm::vec3& origin, const glm::vec3& direction);

    // Zwraca punkt w przestrzeni 3D w odległości t od początku promienia
    glm::vec3 at(float t) const;
};

#endif // RAY_H