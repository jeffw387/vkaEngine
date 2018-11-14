#define STB_TRUETYPE_IMPLEMENTATION
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

std::unique_ptr<BitmapGlyph> Font::loadChar(FT_ULong character) {
  if (FT_Load_Char(face, character, FT_LOAD_RENDER)) {
    MultiLogger::get()->error(
        "Error loading character {} from face {}.",
        character,
        face->family_name);
  }
  return std::make_unique<BitmapGlyph>(face->glyph);
}

std::unique_ptr<BitmapGlyph> Font::loadGlyph(FT_UInt glyphIndex) {
  if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_RENDER)) {
    MultiLogger::get()->error(
        "Error loading glyph index {} from face {}.",
        glyphIndex,
        face->family_name);
  }
  return std::make_unique<BitmapGlyph>(face->glyph);
}

std::vector<FT_ULong> Font::getCharacters() {
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

std::map<FT_ULong, std::unique_ptr<BitmapGlyph>> Font::getGlyphs() {
  std::map<FT_ULong, std::unique_ptr<BitmapGlyph>> result;
  for (auto character : getCharacters()) {
    result[character] = loadChar(character);
  }

  MultiLogger::get()->info("Retrieved glyphs from face.");
  return result;
}

void Font::setPixelSize(uint32_t pixelSize) {
  if (FT_Set_Pixel_Sizes(face, 0, pixelSize)) {
    MultiLogger::get()->error(
        "Error setting font pixel height {} for face {}.",
        pixelSize,
        face->family_name);
  }
  MultiLogger::get()->info("Font pixel height set to {}.", pixelSize);
}

Font::Font(std::string fontPath) {
  MultiLogger::get()->info("Loading font file {} into memory.", fontPath);
  if (auto loadResult = vka::loadBinaryFile({fontPath})) {
    fontBytes = std::move(loadResult.value());
    stbtt_InitFont(&fontInfo, fontBytes.data(), 0);
  }
}

std::unique_ptr<Face> Font::createFace(FT_Long faceIndex) {
  return std::make_unique<Face>(library, fontBytes, faceIndex);
}

Library::Library() {
  if (FT_Init_FreeType(&library)) {
    MultiLogger::get()->error("Error initializing freetype.");
  }
  MultiLogger::get()->info("Initialized freetype.");
}

Library::~Library() {
  MultiLogger::get()->info("Destroying freetype library.");
  FT_Done_FreeType(library);
}

std::unique_ptr<Font> Library::loadFont(std::string fontPath) {
  return std::make_unique<Font>(library, fontPath);
}
}  // namespace Text