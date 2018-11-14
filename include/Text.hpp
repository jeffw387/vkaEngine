// #include <ft2build.h>
// #include FT_FREETYPE_H
// #include FT_GLYPH_H
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
#include "IO.hpp"
#include "Logger.hpp"

namespace Text {

struct Dimensions {
  uint32_t width;
  uint32_t height;
};

template <typename T>
struct Rect {
  T xmin;
  T ymin;
  T xmax;
  T ymax;
};

using Tile = gsl::span<uint8_t>;
auto outputGlyphBitmap = [](const std::vector<uint8_t> &tile,
                            auto tileIndex,
                            Dimensions dimensions) {
  if (tile.size() == 0) return;
  auto fileName = std::string("glyph") + std::to_string(tileIndex) +
                  std::string("-") + std::to_string(dimensions.width) +
                  std::string("x") + std::to_string(dimensions.height) +
                  std::string(".buf");
  std::ofstream outFile{
      fileName,
      std::ios_base::out | std::ios_base::binary | std::ios_base::trunc};
  if (!outFile.is_open()) {
    MultiLogger::get()->error("File {} was not opened!", fileName);
    exit(1);
  }
  outFile.seekp(std::ios::beg);
  auto start = tile.data();
  auto size = tile.size();

  outFile.write((char *)start, size);
  if (outFile.fail()) {
    MultiLogger::get()->error("Write operation to {} failed!", fileName);
    if (outFile.bad()) {
      MultiLogger::get()->error("Bad bit set!");
    }
    exit(1);
  }
};

struct Tileset {
  Tileset(
      ranges::any_view<Tile> tiles,
      uint32_t maxTilesetWidth,
      uint32_t tileWidth,
      uint32_t tileHeight);
  ranges::any_view<Tile> tiles;
  std::vector<Rect<float>> tileUVs;
  std::vector<Rect<int32_t>> tileRects;
  float width = {};
  float height = {};
};

struct BitmapGlyph {
  int32_t xmin;
  int32_t ymin;
  int32_t xmax;
  int32_t ymax;
  uint32_t width;
  uint32_t height;
  int32_t advance;
  std::vector<uint8_t> bitmap;
};

class Font {
public:
  Font(std::string fontPath);
  std::unique_ptr<BitmapGlyph> loadChar(int character);
  std::unique_ptr<BitmapGlyph> loadGlyph(int glyphIndex);
  std::map<int, std::unique_ptr<BitmapGlyph>> getGlyphs();
  void setPixelSize(uint32_t pixelSize);

private:
  std::vector<uint8_t> fontBytes;
  stbtt_fontinfo fontInfo;
  float scale = 1.f;
  std::vector<int> getCharacters();
};
}  // namespace Text