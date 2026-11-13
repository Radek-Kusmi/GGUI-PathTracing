#include "ray.h"

Ray::Ray(const glm::vec3& origin, const glm::vec3& direction)
    : origin(origin), direction(glm::normalize(direction)) {}

glm::vec3 Ray::at(float t) const {
    return origin + t * direction;
}