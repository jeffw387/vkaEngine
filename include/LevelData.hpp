#pragma once
#include <vector>

namespace vka {
struct Chunk {
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