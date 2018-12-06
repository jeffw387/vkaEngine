#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "Asset.hpp"
namespace asset {
void Collection::load(entt::HashedString* const identifier) {
tinygltf::Model gltfModel;
    std::string loadWarning;
    std::string loadError;
    auto loadResult = loader.LoadASCIIFromFile(
        &gltfModel, &loadError, &loadWarning, std::string{static_cast<const char*>(*identifier)});
    if (!loadResult) {
      MultiLogger::get()->error(
          "Error while loading {}: {}", std::string{static_cast<const char*>(*identifier)}, loadError);
    }
    
    asset::Model model{};
    for (auto& node : gltfModel.nodes) {
      Mesh mesh{};
      mesh.name = node.name;
      auto primitive = gltfModel.meshes[node.mesh].primitives.at(0);
      auto indexAccessor = gltfModel.accessors[primitive.indices];
      auto positionAccessor =
          gltfModel.accessors[primitive.attributes["POSITION"]];
      auto normalAccessor = gltfModel.accessors[primitive.attributes["NORMAL"]];
      auto indexBufferView = gltfModel.bufferViews[indexAccessor.bufferView];
      auto positionBufferView =
          gltfModel.bufferViews[positionAccessor.bufferView];
      auto normalBufferView = gltfModel.bufferViews[normalAccessor.bufferView];
      mesh.indexByteOffset = data.size() + indexBufferView.byteOffset;
      mesh.indexCount = indexAccessor.count;
      mesh.positionByteOffset = data.size() + positionBufferView.byteOffset;
      mesh.normalByteOffset = data.size() + normalBufferView.byteOffset;
      if (mesh.name == "Collision") {
        model.collisionMesh = std::move(mesh);
      }
      else {
        model.renderMesh = std::move(mesh);
      }
    }

    data.insert(data.end(), gltfModel.buffers.at(0).data.begin(), gltfModel.buffers.at(0).data.end());
    models[*identifier] = std::move(model);
}

Collection::Collection(tinygltf::TinyGLTF loader, gsl::span<entt::HashedString* const> identifiers) : loader(loader) {
  for (const auto& id : identifiers) {
    load(id);
  }
}
}