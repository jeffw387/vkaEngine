#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <optional>

namespace Camera {
struct Dimensions {
  float left = {};
  float top = {};
  float right = {};
  float bottom = {};
  float nearClip = {};
  float farClip = {};
};

inline auto make_dimensions =
    [](float width, float height, float nearClip = -1.f, float farClip = 1.f) {
      float halfWidth = width * 0.5f;
      float halfHeight = height * 0.5f;
      return Dimensions{
          -halfWidth, halfHeight, halfWidth, -halfHeight, nearClip, farClip};
    };

using Position = glm::vec3;

struct Matrices {
  glm::mat4 view;
  glm::mat4 projection;
};

inline auto mat4_identity = []() { return glm::mat4(1.f); };
inline auto make_view = [](Position position) {
  return glm::translate(mat4_identity(), position);
};

inline auto make_projection = [](Dimensions dimensions) {
  return glm::ortho(
      dimensions.left,
      dimensions.right,
      dimensions.bottom,
      dimensions.top,
      dimensions.nearClip,
      dimensions.farClip);
};
}  // namespace Camera
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