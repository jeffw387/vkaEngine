#define STB_TRUETYPE_IMPLEMENTATION
#include "Text.hpp"

namespace Text {
using namespace ranges;

void Font::setPixelSize(uint32_t pixelSize) {
  scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(pixelSize));

  MultiLogger::get()->info("Font pixel height set to {}.", pixelSize);
}

Font::Font(std::string fontPath) {
  MultiLogger::get()->info("Loading font file {} into memory.", fontPath);
  if (auto loadResult = vka::loadBinaryFile({fontPath})) {
    fontBytes = std::move(loadResult.value());
    stbtt_InitFont(&fontInfo, fontBytes.data(), 0);
  }
}

Atlas Font::getTextureAtlas(int width, int height) {
  Atlas atlas{};
  atlas.pixels.resize(width * height);
  atlas.width = width;
  atlas.height = height;

  stbtt_pack_context packContext{};
  stbtt_PackBegin(
      &packContext, atlas.pixels.data(), width, height, 0, 1, nullptr);
  std::vector<stbtt_packedchar> atlasData;
  atlasData.resize(charSet.characters.size());
  stbtt_PackFontRange(
      &packContext,
      fontBytes.data(),
      0,
      scale,
      32,
      static_cast<int>(charSet.characters.size()),
      atlasData.data());
  stbtt_PackEnd(&packContext);
  RANGES_FOR(auto pair, ranges::view::zip(charSet.characters, atlasData)) {
    atlas.atlasData[std::get<0>(pair)] = std::get<1>(pair);
  }
  return atlas;
}

}  // namespace Text