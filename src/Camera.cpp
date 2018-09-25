#include "Camera.hpp"
#include <glm/glm.hpp>

namespace vka {
void OrthoCamera::setPosition(glm::vec3 position) { this->position = position; }

glm::vec3 OrthoCamera::getPosition() const { return position; }
}  // namespace vka