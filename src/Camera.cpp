#include "Camera.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace vka {
void OrthoCamera::setPosition(glm::vec3 position) {
  this->position = position;
  view.reset();
}

glm::vec3 OrthoCamera::getPosition() const { return position; }

void OrthoCamera::setSides(float left, float top, float right, float bottom) {
  this->left = left;
  this->top = top;
  this->right = right;
  this->bottom = bottom;
  projection.reset();
}

void OrthoCamera::setDimensions(float width, float height) {
  float halfWidth = width * 0.5f;
  float halfHeight = height * 0.5f;
  left = -halfWidth;
  top = halfHeight;
  right = halfWidth;
  bottom = -halfHeight;
  projection.reset();
}

const glm::mat4& OrthoCamera::getView() {
  if (view) {
    return view.value();
  }
  view = glm::translate(glm::mat4(1.f), position);
  return view.value();
}

const glm::mat4& OrthoCamera::getProjection() {
  if (projection) {
    return projection.value();
  }
  projection = glm::ortho(left, right, bottom, top);
  return projection.value();
}
}  // namespace vka