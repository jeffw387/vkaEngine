#pragma once
#include <glm/glm.hpp>
#include <optional>

namespace vka {
class OrthoCamera {
public:
  void setPosition(glm::vec3);
  glm::vec3 getPosition() const;
  void setSides(float left, float top, float right, float bottom);
  void setDimensions(float width, float height);
  void setNearFar(float near, float far);
  const glm::mat4& getView();
  const glm::mat4& getProjection();

private:
  float left = {};
  float top = {};
  float right = {};
  float bottom = {};
  float nearClip = {};
  float farClip = {};
  glm::vec3 position = {};
  std::optional<glm::mat4> view;
  std::optional<glm::mat4> projection;
};
}  // namespace vka