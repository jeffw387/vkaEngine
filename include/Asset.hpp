#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "VulkanFunctionLoader.hpp"

namespace vka {
struct Vertex {
  glm::vec3 positions;
  glm::vec3 normals;
};
struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};
class Asset {
  std::vector<Mesh> meshes;
  VkBuffer assetBuffer;
};
}  // namespace vka