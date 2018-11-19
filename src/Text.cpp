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
  font_size = height;
}

template <>
float Font<>::getScaleFactor() {
  return stbtt_ScaleForPixelHeight(&fontInfo, font_size);
}

template <>
float Font<>::getAdvance(int glyphIndex) {
  int adv{};
  int lsb{};
  stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &adv, &lsb);
  return getScaleFactor() * static_cast<float>(adv);
}

template <>
float Font<>::getKerning(int glyphIndex1, int glyphIndex2) {
  return getScaleFactor() * static_cast<float>(stbtt_GetGlyphKernAdvance(
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
  auto packBeginResult = stbtt_PackBegin(
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
    auto packResult = stbtt_PackFontRange(
        &packContext,
        fontBytes.data(),
        0,
        font_size,
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

template <>
std::vector<stbtt_vertex> Font<>::getGlyphShape(int glyphIndex) {
  stbtt_vertex* vertArray{};
  auto vertCount = stbtt_GetGlyphShape(&fontInfo, glyphIndex, &vertArray);
  std::vector<stbtt_vertex> result;
  result.reserve(vertCount);
  for (int i{}; i < vertCount; ++i) {
    result.push_back(vertArray[i]);
  }
  stbtt_FreeShape(&fontInfo, vertArray);
  return result;
}

// typedef struct {
//   stbtt_vertex_type x, y, cx, cy, cx1, cy1;
//   unsigned char type, padding;
// } stbtt_vertex;

auto addLineEdge = [](msdfgen::Contour& contour,
                      msdfgen::Point2 startPoint,
                      msdfgen::Point2 endPoint) {
  auto& newEdge = contour.addEdge();
  newEdge = msdfgen::EdgeHolder(startPoint, endPoint);
  return endPoint;
};

auto addQuadraticEdge = [](msdfgen::Contour& contour,
                           msdfgen::Point2 startPoint,
                           msdfgen::Point2 controlPoint,
                           msdfgen::Point2 endPoint) {
  auto& newEdge = contour.addEdge();
  newEdge = msdfgen::EdgeHolder(startPoint, controlPoint, endPoint);
  return endPoint;
};

auto addCubicEdge = [](msdfgen::Contour& contour,
                       msdfgen::Point2 startPoint,
                       msdfgen::Point2 controlPoint1,
                       msdfgen::Point2 controlPoint2,
                       msdfgen::Point2 endPoint) {
  auto& newEdge = contour.addEdge();
  newEdge =
      msdfgen::EdgeHolder(startPoint, controlPoint1, controlPoint2, endPoint);
  return endPoint;
};

auto makePoint = [](short x, short y) {
  return msdfgen::Point2{static_cast<double>(x), static_cast<double>(y)};
};

auto makeShape = [](auto stbtt_shape) {
  msdfgen::Shape shape{};
  msdfgen::Point2 nextStartPoint{};
  for (auto& vert : stbtt_shape) {
    switch (vert.type) {
      case STBTT_vmove:
        // MultiLogger::get()->info("Adding new contour.");
        shape.addContour();
        nextStartPoint = makePoint(vert.x, vert.y);
        break;
      case STBTT_vline:
        // MultiLogger::get()->info(
        //     "Creating Line edge: {}x,{}y to {}x,{}y",
        //     nextStartPoint.x,
        //     nextStartPoint.y,
        //     vert.x,
        //     vert.y);
        nextStartPoint = addLineEdge(
            shape.contours.back(), nextStartPoint, makePoint(vert.x, vert.y));
        break;
      case STBTT_vcurve:
        // MultiLogger::get()->info(
        //     "Creating Quadratic edge: {}x,{}y to {}x,{}y",
        //     nextStartPoint.x,
        //     nextStartPoint.y,
        //     vert.x,
        //     vert.y);
        nextStartPoint = addQuadraticEdge(
            shape.contours.back(),
            nextStartPoint,
            makePoint(vert.cx, vert.cy),
            makePoint(vert.x, vert.y));
        break;
      case STBTT_vcubic:
        // MultiLogger::get()->info(
        //     "Creating Cubic edge: {}x,{}y to {}x,{}y",
        //     nextStartPoint.x,
        //     nextStartPoint.y,
        //     vert.x,
        //     vert.y);
        nextStartPoint = addCubicEdge(
            shape.contours.back(),
            nextStartPoint,
            makePoint(vert.cx, vert.cy),
            makePoint(vert.cx1, vert.cy1),
            makePoint(vert.x, vert.y));
        break;
    }
  }
  shape.inverseYAxis = true;
  return shape;
};

auto findCenter = [](double min, double max) { return (max - min) * 0.5; };

// calculate translation to center shape in frame
auto calcTranslation =
    [](double shapeMin, double shapeMax, double frameMin, double frameMax) {
      auto shapeCenter = findCenter(shapeMin, shapeMax);
      auto frameCenter = findCenter(frameMin, frameMax);
      return frameCenter - shapeCenter;
    };
    
template <>
std::unique_ptr<msdfgen::Bitmap<msdfgen::FloatRGB>>
Font<>::getMSDFBitmap(int glyphIndex, int bitmapWidth, int bitmapHeight) {
  auto shape = makeShape(getGlyphShape(glyphIndex));
  if (!shape.validate()) {
    MultiLogger::get()->error(
        "Error: problem with shape from glyph index {}.", glyphIndex);
  }

  msdfgen::edgeColoringSimple(shape, 3);
  shape.normalize();
  double left{};
  double bottom{};
  double right{};
  double top{};
  shape.bounds(left, bottom, right, top);
  auto output = std::make_unique<msdfgen::Bitmap<msdfgen::FloatRGB>>(
      bitmapWidth, bitmapHeight);
  auto vscale = stbtt_ScaleForPixelHeight(&fontInfo, bitmapHeight * 0.75);
  msdfgen::Vector2 scale{vscale, vscale};
  msdfgen::Vector2 translate{
      calcTranslation(left, right, 0, bitmapWidth / vscale),
      calcTranslation(bottom, top, 0, bitmapHeight / vscale)};
  auto range = 2 / vscale;
  msdfgen::generateMSDF(*output, shape, range, scale, translate);

  msdfgen::simulate8bit(*output);
  return output;
}

template <>
MSDFArray Font<>::getMSDFArray(int width, int height) {
  auto charRanges = charSet();
  auto glyphIndexView =
      charRanges | ranges::view::transform([&](auto charRange) {
        return ranges::view::closed_indices(
            charRange.firstChar, charRange.firstChar + charRange.charCount - 1);
      }) |
      ranges::view::join;
  MSDFArray result{};
  ranges::action::push_back(
      result.bitmaps,
      glyphIndexView | ranges::view::transform([&](auto glyphIndex) {
        return getMSDFBitmap(glyphIndex, width, height);
      }));
  return result;
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