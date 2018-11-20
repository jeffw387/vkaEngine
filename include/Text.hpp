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

struct Atlas {
  std::vector<uint8_t> pixels;
  int width = {};
  int height = {};
  std::map<int /*GlyphIndex*/, stbtt_packedchar /*GlyphAtlasData*/> data;

  VertexData getVertexData();

private:
  stbtt_aligned_quad getQuad(int glyphIndex);
  std::vector<stbtt_aligned_quad> getQuads();
};

struct MSDFGlyph {
  msdfgen::Bitmap<msdfgen::FloatRGB> bitmap;
  float left;
  float top;
  float right;
  float bottom;
};

using MSDFGlyphMap = std::map<int, std::unique_ptr<MSDFGlyph>>;

template <typename T = BasicCharacters>
class Font {
public:
  Font(std::string fontPath, int msdfSize = 32, int padding = 2);
  int getGlyphIndex(int charIndex);
  float getAdvance(int glyphIndex, int fontPixelHeight);
  float getKerning(int glyphIndex1, int glyphIndex2, int fontPixelHeight);
  auto getFontBytes() { return fontBytes; }
  VertexData getVertexData();
  std::vector<msdfgen::FloatRGB> getTextureData();
  int getTextureSize() { return msdfSize; }
  int getTextureLayerCount() { return glyphMap.size(); }

private:
  std::vector<stbtt_vertex> getGlyphShape(int glyphIndex);
  std::unique_ptr<MSDFGlyph>
  getMSDFGlyph(int glyphIndex, int bitmapSize, float scaleFactor);
  MSDFGlyphMap getGlyphMap(int bitmapSize, float scaleFactor);

  // multiply scaleFactor by desired font height in pixels
  int msdfSize;
  float scaleFactor;
  std::vector<uint8_t> fontBytes;
  stbtt_fontinfo fontInfo;
  T charSet;
  MSDFGlyphMap glyphMap;
};
}  // namespace Text