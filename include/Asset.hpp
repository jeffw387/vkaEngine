#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <nlohmann/json.hpp>
#include <experimental/filesystem>
#include <fstream>
#include <stdexcept>
#include <memory>
#include "Device.hpp"

namespace vka {
namespace gltf {
namespace fs = std::experimental::filesystem;
using json = nlohmann::json;
struct Node {
  size_t meshIndex;
  std::string name;
};
inline void from_json(const json& j, Node& node) {
  j.at("mesh").get_to(node.meshIndex);
  j.at("name").get_to(node.name);
}
struct Scene {
  std::string name;
  std::vector<size_t> nodeIndices;
};
inline void from_json(const json& j, Scene& scene) {
  j.at("name").get_to(scene.name);
  j.at("nodes").get_to(scene.nodeIndices);
}
struct Primitive {
  size_t positionAccessorIndex;
  size_t normalAccessorIndex;
  size_t indexAccessorIndex;
};
inline void from_json(const json& j, Primitive& primitive) {
  auto attributes = j.at("attributes");
  attributes.at("POSITION").get_to(primitive.positionAccessorIndex);
  attributes.at("NORMAL").get_to(primitive.normalAccessorIndex);
  j.at("indices").get_to(primitive.indexAccessorIndex);
}
struct Mesh {
  std::string name;
  std::vector<Primitive> primitives;
};
inline void from_json(const json& j, Mesh& mesh) {
  j.at("name").get_to(mesh.name);
  j.at("primitives").get_to(mesh.primitives);
}
struct Buffer {
  UniqueAllocatedBuffer vulkanBuffer;
  fs::path uri;
  size_t byteLength;
  std::unique_ptr<char[]> bufferData;
  operator VkBuffer() const { return vulkanBuffer.get().buffer; }
};
inline void from_json(const json& j, Buffer& buffer) {
  j.at("byteLength").get_to(buffer.byteLength);
  j.at("uri").get_to(buffer.uri);
}
struct BufferView {
  size_t bufferIndex;
  size_t byteLength;
  size_t byteOffset;
};
inline void from_json(const json& j, BufferView& view) {
  j.at("buffer").get_to(view.bufferIndex);
  j.at("byteLength").get_to(view.byteLength);
  j.at("byteOffset").get_to(view.byteOffset);
}
struct ComponentType {
  size_t typeCode;

  size_t size() const {
    switch (typeCode) {
      case 5120:
      case 5121:
        return 1;
      case 5122:
      case 5123:
        return 2;
      case 5125:
      case 5126:
        return 4;
      default:
        throw std::logic_error{"Component type code isn't valid!"};
    }
  }
  operator VkIndexType() const {
    switch (typeCode) {
      case 5123:
        return VK_INDEX_TYPE_UINT16;
      case 5125:
        return VK_INDEX_TYPE_UINT32;
      default:
        throw std::logic_error{"Component type code isn't valid!"};
    }
  }
};
inline void from_json(const json& j, ComponentType& componentType) {
  j.get_to(componentType.typeCode);
}
struct ElementType {
  size_t count;
};
inline void from_json(const json& j, ElementType& elementType) {
  std::string typeString = j.get<std::string>();
  if (typeString.compare("SCALAR")) {
    elementType.count = 1;
  } else if (typeString.compare("VEC2")) {
    elementType.count = 2;
  } else if (typeString.compare("VEC3")) {
    elementType.count = 3;
  } else if (typeString.compare("VEC4")) {
    elementType.count = 4;
  } else {
    throw std::logic_error("Element type not valid!");
  }
}

struct Accessor {
  size_t bufferViewIndex;
  ComponentType componentType;
  ElementType elementType;
  size_t elementCount;
};
inline void from_json(const json& j, Accessor& accessor) {
  j.at("bufferView").get_to(accessor.bufferViewIndex);
  j.at("componentType").get_to(accessor.componentType);
  j.at("count").get_to(accessor.elementCount);
  j.at("type").get_to(accessor.elementType);
}
struct Asset {
  std::vector<Scene> scenes;
  size_t defaultScene;
  std::vector<Node> nodes;
  std::vector<Mesh> meshes;
  std::vector<Buffer> buffers;
  std::vector<BufferView> bufferViews;
  std::vector<Accessor> accessors;
};
inline void from_json(const json& j, Asset& asset) {
  j.at("scenes").get_to(asset.scenes);
  j.at("scene").get_to(asset.defaultScene);
  j.at("nodes").get_to(asset.nodes);
  j.at("meshes").get_to(asset.meshes);
  j.at("buffers").get_to(asset.buffers);
  j.at("bufferViews").get_to(asset.bufferViews);
  j.at("accessors").get_to(asset.accessors);
}
inline Asset loadGLTF(fs::path assetPath) {
  auto currentPath = fs::current_path();
  std::ifstream assetFile{assetPath};
  auto assetOpened = assetFile.is_open();
  json j;
  assetFile >> j;
  Asset asset;
  j.get_to(asset);
  for (auto& buffer : asset.buffers) {
    auto binPath = assetPath;
    binPath.replace_filename(buffer.uri);
    std::ifstream binFile{binPath, std::ios_base::in | std::ios_base::binary};
    buffer.bufferData = std::make_unique<char[]>(buffer.byteLength);
    binFile.read(buffer.bufferData.get(), buffer.byteLength);
  }
  return asset;
}

struct BufferData {
  Accessor accessor;
  BufferView view;
  VkBuffer buffer;
};
inline BufferData getBufferData(Asset& asset, size_t accessorIndex) {
  BufferData result{};
  result.accessor = asset.accessors[accessorIndex];
  result.view = asset.bufferViews[result.accessor.bufferViewIndex];
  result.buffer = asset.buffers[result.view.bufferIndex];
  return result;
}

struct VertexBuffers {
  BufferData positionBuffer;
  BufferData normalBuffer;
  BufferData indexBuffer;
};
inline VertexBuffers getVertexBuffers(Asset& asset, size_t nodeIndex) {
  VertexBuffers result{};
  auto meshIndex = asset.nodes[nodeIndex].meshIndex;
  auto positionAccessorIndex =
      asset.meshes[meshIndex].primitives[0].positionAccessorIndex;
  auto normalAccessorIndex =
      asset.meshes[meshIndex].primitives[0].normalAccessorIndex;
  auto indexAccessorIndex =
      asset.meshes[meshIndex].primitives[0].indexAccessorIndex;
  result.positionBuffer = getBufferData(asset, positionAccessorIndex);
  result.normalBuffer = getBufferData(asset, normalAccessorIndex);
  result.indexBuffer = getBufferData(asset, indexAccessorIndex);
  return result;
}
// static void theoreticalTestCase(Asset asset) {
//   VkCommandBuffer cmd{};
//   for (const auto& node : asset.nodes) {
//     for (const auto& primitive : asset.meshes[node.meshIndex].primitives) {
//       const auto& indexAccessor =
//       asset.accessors[primitive.indexAccessorIndex]; const auto&
//       indexBufferView =
//           asset.bufferViews[indexAccessor.bufferViewIndex];
//       const auto& indexBuffer = asset.buffers[indexBufferView.bufferIndex];
//       vkCmdBindIndexBuffer(
//           cmd,
//           indexBuffer,
//           indexBufferView.byteOffset,
//           indexAccessor.componentType);

//       const auto& positionAccessor =
//           asset.accessors[primitive.positionAccessorIndex];
//       const auto& positionBufferView =
//           asset.bufferViews[positionAccessor.bufferViewIndex];
//       const auto& normalAccessor =
//           asset.accessors[primitive.normalAccessorIndex];
//       const auto& normalBufferView =
//           asset.bufferViews[normalAccessor.bufferViewIndex];
//       std::vector<VkBuffer> vertexBuffers = {
//           asset.buffers[positionBufferView.bufferIndex],
//           asset.buffers[normalBufferView.bufferIndex]};
//       std::vector<VkDeviceSize> offsets = {positionBufferView.byteOffset,
//                                            normalBufferView.byteOffset};
//       vkCmdBindVertexBuffers(
//           cmd,
//           0,
//           static_cast<uint32_t>(vertexBuffers.size()),
//           vertexBuffers.data(),
//           offsets.data());
//       vkCmdDrawIndexed(cmd, indexAccessor.elementCount, 1, 0, 0, 0);
//     }
//   }
// }
}  // namespace gltf

}  // namespace vka