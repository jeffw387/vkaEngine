#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <gsl-lite.hpp>

namespace FreeType {

struct BBox {
  int xmin;
  int ymin;
  int xmax;
  int ymax;
};

struct Dimensions {
  int width;
  int height;
};

struct Tile {
  gsl::span<unsigned char> tileData;
  int pitch;
  int rowcount;

  gsl::span<unsigned char> getRow(size_t rowIndex);
};

struct Row {
  std::vector<Tile> tiles;
};

struct Tileset {
  std::vector<Row> rows;
};

inline Tileset makeTileset(std::vector<Tile> tiles, size_t maxTilesetWidth) {
  Tileset result;
  
}

class Glyph {
public:
  Glyph(FT_Glyph glyph);
  ~Glyph();
  gsl::span<unsigned char> render();
  BBox getBoundingBox();
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
  std::vector<std::unique_ptr<Glyph>> getGlyphs();
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
  Library(const Library&) = delete;
  Library& operator=(const Library&) = delete;

  std::unique_ptr<Font> loadFont(std::string fontPath);

private:
  FT_Library library;
};

inline void testLoad() {
  // std::string fontPath = "content/fonts/arial.ttf";
  // Library library;
  // auto font = library.loadFont(fontPath);
  // auto face = font->createFace(0);
}

}  // namespace FreeType