#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <optional>
#include <gsl-lite.hpp>
#include <range/v3/all.hpp>
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

using Tile = gsl::span<unsigned char>;
auto outputGlyphBitmap = [](Tile tile, auto tileIndex, Dimensions dimensions) {
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
  auto size = tile.size_bytes();

  outFile.write((char *)start, size);
  if (outFile.fail()) {
    MultiLogger::get()->error(
        "Write operation to {} failed!", fileName);
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

class Glyph {
public:
  Glyph(FT_Glyph glyph);
  ~Glyph();
  void render();
  Tile getTile();

  Rect<float> getBoundingBox() const;
  Dimensions getDimensions() const;
  int32_t getAdvance();

private:
  FT_Glyph glyph = {};
  FT_BitmapGlyph bitmapGlyph = {};
  std::vector<unsigned char> bitmap;
  bool rendered = false;
};

class Face {
public:
  Face(FT_Library library, std::string fontPath, FT_Long faceIndex);
  ~Face();
  std::unique_ptr<Glyph> loadChar(FT_ULong character);
  std::unique_ptr<Glyph> loadGlyph(FT_UInt glyphIndex);
  std::vector<FT_ULong> getCharacters();
  std::map<FT_ULong, std::unique_ptr<Glyph>> getGlyphs();
  void setSize(uint8_t fontSize, FT_UInt dpi);

private:
  FT_Face face;

  std::unique_ptr<Glyph> getGlyph();
};

class Font {
public:
  Font(FT_Library library, std::string fontPath);
  std::unique_ptr<Face> createFace(FT_Long faceIndex);

private:
  FT_Library library;
  std::string fontPath;
};

class Library {
public:
  Library();
  ~Library();
  Library(const Library &) = delete;
  Library &operator=(const Library &) = delete;

  std::unique_ptr<Font> loadFont(std::string fontPath);

private:
  FT_Library library;
};

}  // namespace Text