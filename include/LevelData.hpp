#pragma once
#include <vector>
#include "Asset.hpp"

namespace vka {
struct Chunk {
  gltf::Asset asset;
};
struct Level {
  std::vector<Chunk> chunks;
  int width;
  int height;
  Chunk getChunk(int x, int y) {
    int rowOffset = width * y;
    int chunkIndex = x + rowOffset;
    return chunks.at(chunkIndex);
  }
};
}  // namespace vka