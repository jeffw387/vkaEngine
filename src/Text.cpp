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

Glyph::~Glyph() { FT_Done_Glyph(glyph); }

void Glyph::render() {
  if (!rendered) {
    if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, 1)) {
      MultiLogger::get()->error("Error rendering glyph.");
    }
    bitmapGlyph = (FT_BitmapGlyph)glyph;
    rendered = true;
  }
}

Tile Glyph::getTile() {
  render();
  auto& bmp = bitmapGlyph->bitmap;
  auto bmpPtr = reinterpret_cast<uint32_t*>(bmp.buffer);
  for (size_t rowIndex{}; rowIndex < bmp.rows; ++rowIndex) {
    auto rowPixels = gsl::span<uint32_t>(bmpPtr, bmp.width);
    ranges::action::push_back(bitmap, rowPixels);
    bmpPtr += bmp.pitch;
}
  return {bitmap};
}

Rect<float> Glyph::getBoundingBox() const {
}

Dimensions Glyph::getDimensions() const {
}

int32_t Glyph::getAdvance() { return glyph->advance.x >> 16; }

Face::Face(FT_Library library, std::string fontPath, FT_Long faceIndex) {
  if (FT_New_Face(library, fontPath.c_str(), faceIndex, &face)) {
    MultiLogger::get()->error(
        "Error creating face {} from font {}.", faceIndex, fontPath);
  }
}

Face::~Face() { FT_Done_Face(face); }

std::unique_ptr<Glyph> Face::getGlyph() {
  FT_Glyph glyph{};
  if (FT_Get_Glyph(face->glyph, &glyph)) {
    MultiLogger::get()->error("Error creating glyph from face.");
  }
  return std::make_unique<Glyph>(glyph);
}

std::unique_ptr<Glyph> Face::loadChar(FT_ULong character) {
  if (FT_Load_Char(face, character, FT_LOAD_DEFAULT)) {
    MultiLogger::get()->error(
        "Error loading character {} from face {}.",
        character,
        face->family_name);
  }
  auto glyph = getGlyph();
  glyph->render();
  return glyph;
}

std::unique_ptr<Glyph> Face::loadGlyph(FT_UInt glyphIndex) {
  if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT)) {
    MultiLogger::get()->error(
        "Error loading glyph index {} from face {}.",
        glyphIndex,
        face->family_name);
  }
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
  if (FT_Set_Char_Size(face, fontSize * 64, 0, dpi, 0)) {
    MultiLogger::get()->error(
        "Error setting font size {} for face {}.", fontSize, face->family_name);
  }
}

Font::Font(FT_Library library, std::string fontPath)
    : library(library), fontPath(fontPath) {}

std::unique_ptr<Face> Font::createFace(FT_Long faceIndex) {
  return std::make_unique<Face>(library, fontPath, faceIndex);
}

Library::Library() {
  if (FT_Init_FreeType(&library)) {
    MultiLogger::get()->error("Error initializing freetype.");
  }
}

Library::~Library() { FT_Done_FreeType(library); }

std::unique_ptr<Font> Library::loadFont(std::string fontPath) {
  return std::make_unique<Font>(library, std::move(fontPath));
}
}  // namespace Text