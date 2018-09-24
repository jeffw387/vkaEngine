#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace vka {
struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
};

struct Mesh {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

struct MeshReference {
  size_t firstIndex;
  size_t vertexOffset;
  size_t indexCount;
  size_t materialIndex;
};

struct Asset {
  std::vector<MeshReference> meshRefs;
};

class AssetBuffer {
public:
  size_t addAsset(
      const std::vector<Mesh>& assetMeshes,
      const std::vector<size_t>& materialIndices) {
    Asset asset{};
    for (auto i = 0U; i < assetMeshes.size(); ++i) {
      auto& assetMesh = assetMeshes[i];
      auto& materialIndex = materialIndices[i];
      MeshReference meshRef{};
      meshRef.firstIndex = mesh.indices.size();
      meshRef.vertexOffset = mesh.vertices.size();
      meshRef.indexCount = assetMesh.indices.size();
      meshRef.materialIndex = materialIndex;
      asset.meshRefs.push_back(meshRef);
      mesh.vertices.insert(
          mesh.vertices.end(),
          assetMesh.vertices.begin(),
          assetMesh.vertices.end());
      mesh.indices.insert(
          mesh.indices.end(),
          assetMesh.indices.begin(),
          assetMesh.indices.end());
    }
    assets.push_back(asset);
    return assets.size();
  }

  constexpr void invalidate() { validBuffer = false; }
  constexpr void validate() { validBuffer = true; }
  bool isValid() { return validBuffer; }
  const Asset& getAsset(size_t assetIndex) { return assets[assetIndex]; }

  VkBuffer vertexBuffer;
  VkBuffer indexBuffer;

private:
  Mesh mesh;
  std::vector<Asset> assets;
  bool validBuffer;
};
}  // namespace vka