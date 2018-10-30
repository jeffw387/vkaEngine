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
#include "Device.hpp"
#include <tiny_gltf.h>
#include "Buffer.hpp"

namespace fs = std::experimental::filesystem;
namespace asset {
struct Model {
  std::string name;
  size_t indexByteOffset;
  size_t indexCount;
  size_t positionByteOffset;
  size_t normalByteOffset;
};

struct Collection {
  std::unique_ptr<vka::Buffer> buffer;
  std::map<uint64_t, Model> models;
};

Collection loadCollection(fs::path);
}  // namespace asset