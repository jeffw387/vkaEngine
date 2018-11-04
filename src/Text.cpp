#include "Text.hpp"

namespace Text {
Tileset::Tileset(std::vector<Tile> tiles, size_t maxTilesetWidth)
    : m_tiles(std::move(tiles)) {
  auto tileWidth = m_tiles.at(0).pitch;
  auto tileHeight = m_tiles.at(0).rowcount;

  int maxTilesPerRow = maxTilesetWidth / tileWidth;
  // range of tiles (range of rows (range of pixels))
  auto bitmapsView = view::transform(m_tiles, [=](Tile tile) {
    return tile.tileData | view::chunk(tileWidth);
  });

  auto tileRows = bitmapsView | view::chunk(maxTilesPerRow);

  float rowIndex{};
  RANGES_FOR(auto row, tileRows) {
    float columnIndex{};
    RANGES_FOR(auto tile, row) {
      tileUVs.push_back({columnIndex * tileWidth,
                         rowIndex * tileHeight,
                         (columnIndex + 1) * tileWidth,
                         (rowIndex + 1) * tileHeight});
      ++columnIndex;
    }
    ++rowIndex;
  }

  // range of rows (range of tiles (range of rows (range of pixels)))
  auto transposedRows =
      tileRows | view::transform([](auto row) { return row | transpose(); });
  auto joinedRows = action::join(transposedRows);
  auto joinedTiles = action::join(joinedRows);
  auto joinedPixelRows = action::join(joinedTiles);
  action::push_back(bitmap, joinedPixelRows);
}

Glyph::Glyph(FT_Glyph glyph) : glyph(glyph){};

Glyph::~Glyph() {
  FT_Done_Glyph(glyph);
  if (rendered) {
    FT_Done_Glyph(FT_Glyph(bitmapGlyph));
  }
}

gsl::span<unsigned char> Glyph::render() {
  if (!rendered) {
    auto newGlyph = glyph;
    FT_Glyph_To_Bitmap(&newGlyph, FT_RENDER_MODE_NORMAL, nullptr, 0);
    bitmapGlyph = (FT_BitmapGlyph)newGlyph;
    rendered = true;
  }
  auto& bmp = bitmapGlyph->bitmap;
  return {bmp.buffer, bmp.pitch * bmp.rows};
}

BBox Glyph::getBoundingBox() {
  FT_BBox bbox{};
  FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_PIXELS, &bbox);
  BBox myBBox{bbox.xMin, bbox.yMin, bbox.xMax, bbox.yMax};
  return myBBox;
}

Dimensions Glyph::getDimensions() {
  auto bbox = getBoundingBox();
  auto width = bbox.xmax - bbox.xmin;
  auto height = bbox.ymax - bbox.ymin;
  return {std::move(width), std::move(height)};
}

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
  return getGlyph();
}

std::unique_ptr<Glyph> Face::loadGlyph(FT_UInt glyphIndex) {
  FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
  return getGlyph();
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

std::vector<std::unique_ptr<Glyph>> Face::getGlyphs() {
  auto chars = getCharacters();
  std::vector<std::unique_ptr<Glyph>> result;
  for (auto character : chars) {
    result.push_back(loadChar(character));
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
}  // namespace FreeType