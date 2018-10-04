#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>

namespace vka {

struct Mesh {
  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<uint32_t> indices;
};

struct MeshReference {
  size_t firstIndex;
  size_t vertexOffset;
  size_t indexCount;
};

namespace gltf {
  struct Node {
    size_t meshIndex;
    std::string name;
  };
  struct Scene {
    std::vector<size_t> nodeIndices;
  };
  struct Primitive {
    size_t positionAccessorIndex;
    size_t normalAccessorIndex;
    size_t indexAccesorIndex;
  };
  struct Mesh {
    std::string name;
    std::vector<Primitive> primitives;
  };
  struct BufferView {
    size_t bufferIndex;
    size_t byteLength;
    size_t byteOffset;
  };
  struct Accessor {
    size_t bufferViewIndex;
    size_t componentSize;
    bool componentSigned;
    size_t componentsPerElement;
    size_t elementCount;
  };
  struct AssetJson {
    std::vector<Scene> scenes;
    size_t defaultScene;
    std::vector<Node> nodes;
    std::vector<Mesh> meshes;
    std::vector<VkBuffer> buffers;
    std::vector<BufferView> bufferViews;
    std::vector<Accessor> accessors;
  };
}

struct Asset {
  std::vector<MeshReference> meshRefs;
};


class AssetBuffer {
public:
  size_t addAsset(
      const std::vector<Mesh>& assetMeshes) {
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

  VkBuffer data;

private:
  Mesh mesh;
  std::vector<Asset> assets;
  bool validBuffer;
};


}  // namespace vka