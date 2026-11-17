#include "ray.h"

Ray::Ray(const glm::vec3& origin, const glm::vec3& direction, bool normalizeDir)
    : origin(origin) {
    this->direction = normalizeDir ? glm::normalize(direction) : direction;

    // Prekalkulacja odwrotności kierunku (zastępuje wolne dzielenie szybkim mnożeniem w testach AABB)
    this->invDir = glm::vec3(
        1.0f / this->direction.x,
        1.0f / this->direction.y,
        1.0f / this->direction.z
    );
}

glm::vec3 Ray::at(float t) const {
    return origin + t * direction;
}