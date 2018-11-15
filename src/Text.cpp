#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "Text.hpp"

namespace Text {
using namespace ranges;
template <>
Font<>::Font(std::string fontPath) {
  MultiLogger::get()->info("Loading font file {} into memory.", fontPath);
  if (auto loadResult = vka::loadBinaryFile({fontPath})) {
    fontBytes = std::move(loadResult.value());
    stbtt_InitFont(&fontInfo, fontBytes.data(), 0);
  }
}

template <>
void Font<>::setFontPixelHeight(uint32_t height) {
  scale = stbtt_ScaleForPixelHeight(&fontInfo, static_cast<float>(height));
}

template <>
float Font<>::getAdvance(int glyphIndex) {
  int adv{};
  int lsb{};
  stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &adv, &lsb);
  return scale * static_cast<float>(adv);
}

template <>
float Font<>::getKerning(int glyphIndex1, int glyphIndex2) {
  return scale * static_cast<float>(stbtt_GetGlyphKernAdvance(
                     &fontInfo, glyphIndex1, glyphIndex2));
}

template <>
int Font<>::getGlyphIndex(int charIndex) {
  return stbtt_FindGlyphIndex(&fontInfo, charIndex);
}

template <>
Atlas Font<>::getTextureAtlas(int atlasWidth, int atlasHeight) {
  Atlas atlas{};
  atlas.pixels.resize(atlasWidth * atlasHeight);
  atlas.width = atlasWidth;
  atlas.height = atlasHeight;

  stbtt_pack_context packContext{};
  stbtt_PackBegin(
      &packContext,
      atlas.pixels.data(),
      atlasWidth,
      atlasHeight,
      0,
      1,
      nullptr);
  std::vector<std::vector<stbtt_packedchar>> atlasDatas;

  auto charRanges = charSet();

  for (auto charRange : charRanges) {
  std::vector<stbtt_packedchar> atlasData;
    atlasData.resize(charRange.charCount);
  stbtt_PackFontRange(
      &packContext,
      fontBytes.data(),
      0,
      scale,
        charRange.firstChar,
        static_cast<int>(charRange.charCount),
      atlasData.data());
    atlasDatas.push_back(std::move(atlasData));
  }
  stbtt_PackEnd(&packContext);
  auto glyphIndicesView =
      charRanges | ranges::view::transform([](auto charRange) {
        return ranges::view::closed_indices(
            static_cast<size_t>(charRange.firstChar),
            charRange.firstChar + charRange.charCount);
      }) |
      ranges::view::join | ranges::view::transform([this](auto charIndex) {
        return getGlyphIndex(charIndex);
      });
  auto atlasDataView = atlasDatas | ranges::view::join;
  RANGES_FOR(auto pair, ranges::view::zip(glyphIndicesView, atlasDataView)) {
    atlas.data[std::get<0>(pair)] = std::get<1>(pair);
  }
  return atlas;
}

stbtt_aligned_quad Atlas::getQuad(int glyphIndex) {
  stbtt_aligned_quad quad{};
  float xAdvance{};
  float yAdvance{};
  stbtt_GetPackedQuad(
      &data[glyphIndex], width, height, 0, &xAdvance, &yAdvance, &quad, 0);
  return quad;
}

std::vector<stbtt_aligned_quad> Atlas::getQuads() {
  std::vector<stbtt_aligned_quad> result;
  ranges::action::push_back(
      result, ranges::view::transform(data, [this](auto pair) {
        return getQuad(pair.first);
      }));
  return result;
}

VertexData Atlas::getVertexData() {
  VertexData result{};
  size_t indexOffset{};
  size_t vertexOffset{};
  auto quads = getQuads();
  auto zipView = ranges::view::zip(ranges::view::keys(data), quads);
  RANGES_FOR(auto zipTuple, zipView) {
    auto& quad = std::get<1>(zipTuple);
    auto left = quad.x0;
    auto top = quad.y0;
    auto right = quad.x1;
    auto bottom = quad.y1;
    auto leftuv = quad.s0;
    auto topuv = quad.t0;
    auto rightuv = quad.s1;
    auto bottomuv = quad.t1;

    ranges::action::push_back(
        result.indices,
        {vertexOffset + 0,
         vertexOffset + 1,
         vertexOffset + 2,
         vertexOffset + 2,
         vertexOffset + 3,
         vertexOffset + 0});
    ranges::action::push_back(
        result.vertices,
        std::vector<Vertex>{{{left, top}, {leftuv, topuv}},
                            {{left, bottom}, {leftuv, bottomuv}},
                            {{right, bottom}, {rightuv, bottomuv}},
                            {{right, top}, {rightuv, topuv}}});
    result.offsets[std::get<0>(zipTuple)] = indexOffset;

    indexOffset += IndicesPerQuad;
    vertexOffset += VerticesPerQuad;
  }
  return result;
}
}  // namespace Text