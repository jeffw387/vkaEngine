#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "Text.hpp"

namespace Text {
using namespace ranges;

template <>
float Font<>::getAdvance(int glyphIndex, int fontPixelHeight) {
  int adv{};
  int lsb{};
  stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &adv, &lsb);
  return scaleFactor * fontPixelHeight * static_cast<float>(adv);
}

template <>
float Font<>::getKerning(
    int glyphIndex1,
    int glyphIndex2,
    int fontPixelHeight) {
  return scaleFactor * fontPixelHeight *
         static_cast<float>(
             stbtt_GetGlyphKernAdvance(&fontInfo, glyphIndex1, glyphIndex2));
}

template <>
int Font<>::getGlyphIndex(int charIndex) {
  return stbtt_FindGlyphIndex(&fontInfo, charIndex);
}

template <>
VertexData Font<>::getVertexData() {
  namespace view = ranges::view;
  namespace action = ranges::action;
  VertexData result{};
  size_t indexOffset{};
  size_t vertexOffset{};
  for (const auto& [glyphIndex, bitmap] : glyphMap) {
    action::push_back(
        result.indices,
        std::vector<Index>{
            static_cast<uint16_t>(vertexOffset + 0),
            static_cast<uint16_t>(vertexOffset + 1),
            static_cast<uint16_t>(vertexOffset + 2),
            static_cast<uint16_t>(vertexOffset + 2),
            static_cast<uint16_t>(vertexOffset + 3),
            static_cast<uint16_t>(vertexOffset + 0),
        });
    auto l = bitmap->left;
    auto t = bitmap->top;
    auto r = bitmap->right;
    auto b = bitmap->bottom;
    action::push_back(
        result.vertices,
        std::vector<Vertex>{{{l, t}, {0, 0}},
                            {{l, b}, {0, 1}},
                            {{r, b}, {1, 1}},
                            {{r, t}, {1, 0}}});
    result.offsets[glyphIndex] = indexOffset;
    indexOffset += IndicesPerQuad;
    vertexOffset += VerticesPerQuad;
  }
  return result;
}

template <>
std::vector<msdfgen::FloatRGB> Font<>::getTextureData() {
  namespace view = ranges::view;
  namespace action = ranges::action;
  auto pixelRanges =
      glyphMap | view::values | view::transform([](auto& bitmapPtr) {
        auto& bmp = bitmapPtr->bitmap;
        return gsl::span<msdfgen::FloatRGB>(
            bmp.data(), bmp.width() * bmp.height());
      });
  auto pixelsView = pixelRanges | view::join;
  std::vector<msdfgen::FloatRGB> result;
  action::push_back(result, pixelsView);
  return result;
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
        shape.addContour();
        nextStartPoint = makePoint(vert.x, vert.y);
        break;
      case STBTT_vline:
        nextStartPoint = addLineEdge(
            shape.contours.back(), nextStartPoint, makePoint(vert.x, vert.y));
        break;
      case STBTT_vcurve:
        nextStartPoint = addQuadraticEdge(
            shape.contours.back(),
            nextStartPoint,
            makePoint(vert.cx, vert.cy),
            makePoint(vert.x, vert.y));
        break;
      case STBTT_vcubic:
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
std::unique_ptr<MSDFGlyph>
Font<>::getMSDFGlyph(int glyphIndex, int bitmapSize, float scaleFactor) {
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
  auto output = msdfgen::Bitmap<msdfgen::FloatRGB>(bitmapSize, bitmapSize);
  msdfgen::Vector2 scale{scaleFactor, scaleFactor};
  msdfgen::Vector2 translate{
      calcTranslation(left, right, 0, bitmapSize / scaleFactor),
      calcTranslation(bottom, top, 0, bitmapSize / scaleFactor)};
  auto range = 2 / scaleFactor;
  msdfgen::generateMSDF(output, shape, range, scale, translate);

  // TODO: is this needed? Maybe a better way to store output?
  // msdfgen::simulate8bit(output);

  auto left_trans = static_cast<float>(left + translate.x) * scaleFactor;
  auto top_trans = static_cast<float>(-(top + translate.y)) * scaleFactor;
  auto right_trans = static_cast<float>(right + translate.x) * scaleFactor;
  auto bottom_trans = static_cast<float>(-(bottom + translate.y)) * scaleFactor;
  return std::make_unique<MSDFGlyph>(MSDFGlyph{
      std::move(output), left_trans, top_trans, right_trans, bottom_trans});
}

template <>
MSDFGlyphMap Font<>::getGlyphMap(int bitmapSize, float scaleFactor) {
  auto charRanges = charSet();
  auto charIndexView =
      charRanges | ranges::view::transform([&](auto charRange) {
        return ranges::view::closed_indices(
            charRange.firstChar, charRange.firstChar + charRange.charCount - 1);
      }) |
      ranges::view::join;
  MSDFGlyphMap result{};
  RANGES_FOR(auto charIndex, charIndexView) {
    auto glyphIndex = getGlyphIndex(charIndex);
    result[glyphIndex] = getMSDFGlyph(glyphIndex, bitmapSize, scaleFactor);
  }
  return result;
}

template <>
Font<>::Font(std::string fontPath, int msdfSize, int padding)
    : msdfSize(msdfSize) {
  MultiLogger::get()->info("Loading font file {} into memory.", fontPath);
  if (auto loadResult = vka::loadBinaryFile({fontPath})) {
    fontBytes = std::move(loadResult.value());
    stbtt_InitFont(&fontInfo, fontBytes.data(), 0);
  }
  auto scaledGlyphHeight = msdfSize - (padding * 2);
  scaleFactor = stbtt_ScaleForPixelHeight(&fontInfo, scaledGlyphHeight);
  glyphMap = getGlyphMap(msdfSize, scaleFactor);
  scaleFactor /= msdfSize;
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