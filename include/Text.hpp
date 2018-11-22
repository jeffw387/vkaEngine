#include <stb_rect_pack.h>
#include <stb_truetype.h>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <optional>
#include <gsl-lite.hpp>
#include <range/v3/all.hpp>
#include <stdexcept>
#include <glm/glm.hpp>
#include <optional>
#include <msdfgen.h>
#include "IO.hpp"
#include "Logger.hpp"

namespace Text {
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

struct MSDFGlyph {
  msdfgen::Bitmap<msdfgen::FloatRGB> bitmap;
  struct Pos {
    float left;
    float top;
    float right;
    float bottom;
  } pos;
  struct UV {
    float left;
    float top;
    float right;
    float bottom;
  } uv;
  uint32_t arrayIndex;
};

using MSDFGlyphMap = std::map<int, std::unique_ptr<MSDFGlyph>>;

template <typename T = BasicCharacters>
class Font {
public:
  Font(std::string fontPath, int msdfSize = 64, int padding = 2);
  int getGlyphIndex(int charIndex);
  uint32_t getArrayIndex(int glyphIndex);
  float getAdvance(int glyphIndex, int fontPixelHeight);
  float getKerning(int glyphIndex1, int glyphIndex2, int fontPixelHeight);
  float msdfToRenderRatio(int fontPixelHeight);
  float vectorToRenderRatio(int fontPixelHeight);
  auto getFontBytes() { return fontBytes; }
  std::unique_ptr<VertexData> getVertexData();
  std::vector<uint8_t> getTextureData();
  int getTextureSize() { return msdfSize; }
  int getTextureLayerCount() { return glyphMap.size(); }

private:
  std::vector<stbtt_vertex> getGlyphShape(int glyphIndex);
  std::unique_ptr<MSDFGlyph>
  getMSDFGlyph(int glyphIndex, int bitmapSize, float scaleFactor);
  MSDFGlyphMap getGlyphMap(int bitmapSize, float scaleFactor);

  int msdfSize;
  int originalPixelHeight;
  std::vector<uint8_t> fontBytes;
  stbtt_fontinfo fontInfo;
  T charSet;
  MSDFGlyphMap glyphMap;
};
}  // namespace Text