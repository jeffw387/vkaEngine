#pragma once
#include <vector>
#include <string>
#include <cstring>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <nlohmann/json.hpp>
#include <experimental/filesystem>
#include <fstream>
#include <stdexcept>
#include <memory>
#include <optional>
#include <tiny_gltf.h>
#include "gsl-lite.hpp"
#include "entt/entt.hpp"
#include "Device.hpp"
#include "Buffer.hpp"

namespace fs = std::experimental::filesystem;
namespace asset {
struct Mesh {
  std::string name;
  size_t indexByteOffset;
  size_t indexCount;
  size_t positionByteOffset;
  size_t normalByteOffset;
};

struct Model {
  std::optional<Mesh> renderMesh;
  std::optional<Mesh> collisionMesh;
};

struct Collection {
  tinygltf::TinyGLTF loader;
  std::vector<uint8_t> data;
  std::shared_ptr<vka::Buffer> buffer;
  std::map<uint64_t, Model> models;

  Collection(tinygltf::TinyGLTF loader, gsl::span<entt::HashedString* const> identifiers);
private:
  void load(entt::HashedString* const identifier);
};


}  // namespace asset