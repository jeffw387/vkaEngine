#pragma once
#include <stb_rect_pack.h>
#include <stb_truetype.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <optional>
#include <range/v3/all.hpp>
#include <stdexcept>
#include <glm/glm.hpp>
#include <optional>
#include <msdfgen.h>
#include "gsl-lite.hpp"
#include "io.hpp"
#include "logger.hpp"

namespace Text {
namespace fs = std::experimental::filesystem;
using Index = uint16_t;
struct Vertex {
  glm::vec2 pos = {};
  glm::vec2 uv = {};
};
constexpr uint8_t IndicesPerQuad = 6;
constexpr uint8_t VerticesPerQuad = 4;

struct CharacterRange {
  size_t firstChar = {};
  size_t charCount = {};
};

struct BasicCharacters {
  CharacterRange basicSet = {32, 127 - 32 /*TODO: could be off-by-one*/};

  std::vector<CharacterRange> operator()() { return {basicSet}; }
};

struct VertexData {
  std::vector<Index> indices;
  std::vector<Vertex> vertices;
  std::map<uint8_t /*GlyphIndex*/, size_t /*IndexOffset*/> offsets;
};

template <typename T>
struct Rect {
  T xmin;
  T ymin;
  T xmax;
  T ymax;
};

struct MSDFGlyph {
  msdfgen::Bitmap<msdfgen::FloatRGB> bitmap;
  Rect<float> pos;
  Rect<float> uv;
  uint32_t arrayIndex;
};

using MSDFGlyphMap = std::map<int, std::unique_ptr<MSDFGlyph>>;

template <typename T = BasicCharacters>
class Font {
public:
  Font(fs::path fontPath, int bitmapSize = 64, int padding = 2);
  int getGlyphIndex(int charIndex);
  uint32_t getArrayIndex(int glyphIndex);
  float getAdvance(int glyphIndex, int fontPixelHeight);
  float getKerning(int glyphIndex1, int glyphIndex2, int fontPixelHeight);
  float vectorToRenderRatio(int fontPixelHeight);
  std::unique_ptr<VertexData> getVertexData();
  std::vector<uint8_t> getTextureData();
  auto getFontBytes() { return fontBytes; }
  int getLayerCount() { return glyphMap.size(); }
  int getBitmapSize() { return bitmapSize; }
  int getOriginalPixelHeight() { return originalPixelHeight; }
  float getScaleFactor() { return scaleFactor; }
  float getPixelRange() { return pixelRange; }

private:
  std::vector<stbtt_vertex> getGlyphShape(int glyphIndex);
  std::unique_ptr<MSDFGlyph> getMSDFGlyph(int glyphIndex, int bitmapSize);
  MSDFGlyphMap getGlyphMap(int bitmapSize);

  int bitmapSize;
  int originalPixelHeight;
  int padding;
  float scaleFactor;
  const float pixelRange = 2.f;
  std::vector<uint8_t> fontBytes;
  stbtt_fontinfo fontInfo;
  T charSet;
  MSDFGlyphMap glyphMap;
};
}  // namespace Text