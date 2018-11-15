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
#include "IO.hpp"
#include "Logger.hpp"

namespace Text {

// using Tile = gsl::span<uint8_t>;
// auto outputGlyphBitmap = [](const std::vector<uint8_t> &tile,
//                             auto tileIndex,
//                             Dimensions dimensions) {
//   if (tile.size() == 0) return;
//   auto fileName = std::string("glyph") + std::to_string(tileIndex) +
//                   std::string("-") + std::to_string(dimensions.width) +
//                   std::string("x") + std::to_string(dimensions.height) +
//                   std::string(".buf");
//   std::ofstream outFile{
//       fileName,
//       std::ios_base::out | std::ios_base::binary | std::ios_base::trunc};
//   if (!outFile.is_open()) {
//     MultiLogger::get()->error("File {} was not opened!", fileName);
//     exit(1);
//   }
//   outFile.seekp(std::ios::beg);
//   auto start = tile.data();
//   auto size = tile.size();

//   outFile.write((char *)start, size);
//   if (outFile.fail()) {
//     MultiLogger::get()->error("Write operation to {} failed!", fileName);
//     if (outFile.bad()) {
//       MultiLogger::get()->error("Bad bit set!");
//     }
//     exit(1);
//   }
// };
using Index = uint16_t;
struct Vertex {
  glm::vec2 pos = {};
  glm::vec2 uv = {};
};
constexpr uint8_t IndicesPerQuad = 6;
constexpr uint8_t VerticesPerQuad = 4;

struct CharacterRange {
  int firstChar = {};
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
  std::map<uint8_t /*GlyphIndex*/, stbtt_packedchar /*GlyphAtlasData*/> data;

  VertexData getVertexData();

private:
  stbtt_aligned_quad getQuad(int glyphIndex);
  std::vector<stbtt_aligned_quad> getQuads();
};

template <typename T = BasicCharacters>
class Font {
public:
  Font(std::string fontPath);
  int getGlyphIndex(int charIndex);
  Atlas getTextureAtlas(int width, int height);
  void setFontPixelHeight(uint32_t height);
  float getAdvance(int glyphIndex);
  float getKerning(int glyphIndex1, int glyphIndex2);

private:
  std::vector<uint8_t> fontBytes;
  stbtt_fontinfo fontInfo;
  T charSet;
  float scale = 1.f;
};
}  // namespace Text