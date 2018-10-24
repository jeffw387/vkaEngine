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

namespace vka {
namespace fs = std::experimental::filesystem;
namespace gltf {
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

}  // namespace gltf

namespace asset {
struct Model {
  std::string name;
  size_t indexByteOffset;
  size_t indexCount;
  size_t positionByteOffset;
  size_t normalByteOffset;
};
struct Collection {
  UniqueAllocatedBuffer buffer;
  std::map<uint64_t, Model> models;
};

inline Collection loadCollection(Device* device, fs::path assetPath) {
  auto gltfAsset = gltf::loadGLTF(assetPath);
  Collection result;
  auto nodeIndex = 0U;
  for (auto& node : gltfAsset.nodes) {
    auto vertBuffers = gltf::getVertexBuffers(gltfAsset, nodeIndex);
    Model model{};
    model.name = node.name;
    model.indexByteOffset = vertBuffers.indexBuffer.view.byteOffset;
    model.indexCount = vertBuffers.indexBuffer.accessor.elementCount;
    model.positionByteOffset = vertBuffers.positionBuffer.view.byteOffset;
    model.normalByteOffset = vertBuffers.normalBuffer.view.byteOffset;
    result.models[nodeIndex] = std::move(model);
    ++nodeIndex;
  }
  result.buffer = device->createAllocatedBuffer(
      gltfAsset.buffers.at(0).byteLength,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
          VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VMA_MEMORY_USAGE_GPU_ONLY);
  auto stagingBuffer = device->createAllocatedBuffer(
      gltfAsset.buffers.at(0).byteLength,
      VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
      VMA_MEMORY_USAGE_CPU_ONLY);

  auto byteLength = gltfAsset.buffers[0].byteLength;
  auto copyFence = device->createFence(false);

  void* stagePtr{};
  vmaMapMemory(
      device->getAllocator(), stagingBuffer.get().allocation, &stagePtr);
  std::memcpy(stagePtr, gltfAsset.buffers[0].bufferData.get(), byteLength);
  vmaFlushAllocation(
      device->getAllocator(), stagingBuffer.get().allocation, 0, byteLength);
  auto cmdPool = device->createCommandPool();
  auto cmd = cmdPool.allocateCommandBuffer();
  cmd->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
  cmd->copyBuffer(
      stagingBuffer.get().buffer,
      result.buffer.get().buffer,
      {{0U, 0U, byteLength}});
  cmd->end();
  device->queueSubmit({}, {*cmd}, {}, copyFence);
  copyFence.wait();
  return std::move(result);
}
}  // namespace asset

}  // namespace vka