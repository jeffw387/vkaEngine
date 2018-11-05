#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <optional>
#include <gsl-lite.hpp>
#include "Logger.hpp"
#include <range/v3/all.hpp>

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

struct Tileset {
  Tileset(
      ranges::any_view<Tile> tiles,
      uint32_t maxTilesetWidth,
      uint32_t tileWidth,
      uint32_t tileHeight);
  ranges::any_view<Tile> tiles;
  std::vector<Rect<float>> tileUVs;
  std::vector<Rect<uint32_t>> tileRects;
  float width = {};
  float height = {};
};

class Glyph {
public:
  Glyph(FT_Glyph glyph);
  ~Glyph();
  void render();
  Tile getTile();

  Rect<int> getBoundingBox();
  Dimensions getDimensions();

private:
  FT_Glyph glyph;
  FT_BitmapGlyph bitmapGlyph;
  bool rendered;
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