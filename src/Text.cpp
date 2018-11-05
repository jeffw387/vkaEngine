#include "Text.hpp"

namespace Text {
using namespace ranges;

Tileset::Tileset(
    any_view<Tile> tiles,
    uint32_t maxTilesetWidth,
    uint32_t tileWidth,
    uint32_t tileHeight)
    : tiles(tiles) {
  int maxTilesPerRow = maxTilesetWidth / tileWidth;

  auto tileRows = tiles | view::chunk(maxTilesPerRow);
  auto tileRowCount = distance(tileRows);
  width = maxTilesetWidth;
  height = tileRowCount * tileHeight;

  float rowIndex{};
  RANGES_FOR(auto row, tileRows) {
    float columnIndex{};
    RANGES_FOR(auto tile, row) {
      tileRects.push_back({static_cast<int32_t>(columnIndex * tileWidth),
                           static_cast<int32_t>(rowIndex * tileHeight),
                           static_cast<int32_t>((columnIndex + 1) * tileWidth),
                           static_cast<int32_t>((rowIndex + 1) * tileHeight)});
      tileUVs.push_back({(columnIndex * tileWidth) / width,
                         (rowIndex * tileHeight) / height,
                         ((columnIndex + 1) * tileWidth) / width,
                         ((rowIndex + 1) * tileHeight) / height});
      ++columnIndex;
    }
    ++rowIndex;
  }
}

Glyph::Glyph(FT_Glyph glyph) : glyph(glyph){};

Glyph::~Glyph() {
  FT_Done_Glyph(glyph);
  if (rendered) {
    FT_Done_Glyph(FT_Glyph(bitmapGlyph));
  }
}

void Glyph::render() {
  if (!rendered) {
    auto newGlyph = glyph;
    FT_Glyph_To_Bitmap(&newGlyph, FT_RENDER_MODE_NORMAL, nullptr, 0);
    bitmapGlyph = (FT_BitmapGlyph)newGlyph;
    rendered = true;
  }
}

Tile Glyph::getTile() {
  render();
  auto& bmp = bitmapGlyph->bitmap;
  return {bmp.buffer, bmp.width * bmp.rows};
}

Rect<float> Glyph::getBoundingBox() {
  FT_BBox bbox{};
  FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
  Rect<float> myBBox{static_cast<float>(bbox.xMin),
                     static_cast<float>(bbox.yMin),
                     static_cast<float>(bbox.xMax),
                     static_cast<float>(bbox.yMax)};
  return myBBox;
}

Dimensions Glyph::getDimensions() {
  auto bbox = getBoundingBox();
  uint32_t width = std::abs(bbox.xmax - bbox.xmin);
  uint32_t height = std::abs(bbox.ymax - bbox.ymin);
  return {std::move(width), std::move(height)};
}

int32_t Glyph::getAdvance() { return glyph->advance.x >> 16; }

Face::Face(FT_Library library, std::string fontPath, FT_Long faceIndex) {
  FT_New_Face(library, fontPath.c_str(), faceIndex, &face);
}

Face::~Face() { FT_Done_Face(face); }

std::unique_ptr<Glyph> Face::getGlyph() {
  FT_Glyph glyph{};
  FT_Get_Glyph(face->glyph, &glyph);
  return std::make_unique<Glyph>(glyph);
}

std::unique_ptr<Glyph> Face::loadChar(FT_ULong character) {
  FT_Load_Char(face, character, FT_LOAD_DEFAULT);
  auto glyph = getGlyph();
  glyph->render();
  return glyph;
}

std::unique_ptr<Glyph> Face::loadGlyph(FT_UInt glyphIndex) {
  FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
  auto glyph = getGlyph();
  glyph->render();
  return glyph;
}

std::vector<FT_ULong> Face::getCharacters() {
  FT_UInt glyphIndex{};
  std::vector<FT_ULong> result;
  result.push_back(FT_Get_First_Char(face, &glyphIndex));

  while (true) {
    result.push_back(FT_Get_Next_Char(face, result.back(), &glyphIndex));
    if (!glyphIndex) {
      break;
    }
  }
  return result;
}

std::map<FT_ULong, std::unique_ptr<Glyph>> Face::getGlyphs() {
  std::map<FT_ULong, std::unique_ptr<Glyph>> result;
  for (auto character : getCharacters()) {
    result[character] = loadChar(character);
  }
  return result;
}

void Face::setSize(uint8_t fontSize, FT_UInt dpi) {
  FT_Set_Char_Size(face, fontSize * 64, 0, dpi, 0);
}

Font::Font(FT_Library library, std::string fontPath)
    : library(library), fontPath(fontPath) {}

std::unique_ptr<Face> Font::createFace(FT_Long faceIndex) {
  return std::make_unique<Face>(library, fontPath, faceIndex);
}

Library::Library() { FT_Init_FreeType(&library); }

Library::~Library() { FT_Done_FreeType(library); }

std::unique_ptr<Font> Library::loadFont(std::string fontPath) {
  return std::make_unique<Font>(library, std::move(fontPath));
}
}  // namespace Text