#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "Text.hpp"

namespace Text {
using namespace ranges;

template <>
float Font<>::vectorToRenderRatio(int fontPixelHeight) {
  return stbtt_ScaleForPixelHeight(&fontInfo, fontPixelHeight);
}

template <>
float Font<>::getAdvance(int glyphIndex, int fontPixelHeight) {
  int adv{};
  int lsb{};
  stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &adv, &lsb);
  return static_cast<float>(adv);
}

template <>
float Font<>::getKerning(
    int glyphIndex1,
    int glyphIndex2,
    int fontPixelHeight) {
  auto unscaledKern = static_cast<float>(
      stbtt_GetGlyphKernAdvance(&fontInfo, glyphIndex1, glyphIndex2));
  return unscaledKern;
}

template <>
int Font<>::getGlyphIndex(int charIndex) {
  return stbtt_FindGlyphIndex(&fontInfo, charIndex);
}

template <>
uint32_t Font<>::getArrayIndex(int glyphIndex) {
  return glyphMap[glyphIndex]->arrayIndex;
}

template <>
std::unique_ptr<VertexData> Font<>::getVertexData() {
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
    auto pos = bitmap->pos;
    auto uv = bitmap->uv;
    action::push_back(
        result.vertices,
        std::vector<Vertex>{{{pos.xmin, pos.ymin}, {uv.xmin, uv.ymin}},
                            {{pos.xmin, pos.ymax}, {uv.xmin, uv.ymax}},
                            {{pos.xmax, pos.ymax}, {uv.xmax, uv.ymax}},
                            {{pos.xmax, pos.ymin}, {uv.xmax, uv.ymin}}});
    result.offsets[glyphIndex] = indexOffset;
    indexOffset += IndicesPerQuad;
    vertexOffset += VerticesPerQuad;
  }
  return std::make_unique<VertexData>(result);
}

auto F32ToU8 = [](msdfgen::FloatRGB F32Pixel) {
  std::vector<uint8_t> result;
  uint8_t r = msdfgen::clamp(int(F32Pixel.r * 0x100), 0xff);
  uint8_t g = msdfgen::clamp(int(F32Pixel.g * 0x100), 0xff);
  uint8_t b = msdfgen::clamp(int(F32Pixel.b * 0x100), 0xff);
  uint8_t a = {};
  result.push_back(r);
  result.push_back(g);
  result.push_back(b);
  result.push_back(a);
  return result;
};

template <>
std::vector<uint8_t> Font<>::getTextureData() {
  namespace view = ranges::view;
  namespace action = ranges::action;
  auto pixelRanges =
      glyphMap | view::values | view::transform([](auto& bitmapPtr) {
        auto& bmp = bitmapPtr->bitmap;
        return gsl::span<msdfgen::FloatRGB>(
            bmp.data(), bmp.width() * bmp.height());
      });
  auto pixelsView = pixelRanges | view::join;
  std::vector<uint8_t> result;
  RANGES_FOR(auto pixel, pixelsView) {
    action::push_back(result, F32ToU8(pixel));
  }
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

template <typename T>
auto findCenter = [](T min, T max) { return ((max - min) * 0.5) + min; };

// calculate translation to center shape in frame
template <typename T>
auto calcTranslation = [](T shapeMin, T shapeMax, T frameMin, T frameMax) {
  auto shapeCenter = findCenter<T>(shapeMin, shapeMax);
  auto frameCenter = findCenter<T>(frameMin, frameMax);
  return frameCenter - shapeCenter;
};

auto getGlyphRenderBounds = [](Rect<double> shapeBounds, float scaleFactor) {
  Rect<float> result = {static_cast<float>(shapeBounds.xmin * scaleFactor),
                        static_cast<float>(shapeBounds.ymin * scaleFactor),
                        static_cast<float>(shapeBounds.xmax * scaleFactor),
                        static_cast<float>(shapeBounds.ymax * scaleFactor)};
  return result;
};

auto getTransformedRenderBounds =
    [](Rect<float> renderBounds, float xOffset, float yOffset) {
      renderBounds.xmin += xOffset;
      renderBounds.xmax += xOffset;
      renderBounds.ymin += yOffset;
      renderBounds.ymax += yOffset;
      return renderBounds;
    };

auto getUV = [](Rect<float> transformedBounds, float size) {
  transformedBounds.xmin /= size;
  transformedBounds.xmax /= size;
  transformedBounds.ymin /= size;
  transformedBounds.ymax /= size;
  return transformedBounds;
};

auto flipY = [](auto original) {
  std::swap(original.ymin, original.ymax);
  original.ymin = -original.ymin;
  original.ymax = -original.ymax;
  return original;
};

auto padRect = [](auto original, auto padding) {
  original.xmin -= padding;
  original.ymin -= padding;
  original.xmax += padding;
  original.ymax += padding;
  return original;
};

template <>
std::unique_ptr<MSDFGlyph> Font<>::getMSDFGlyph(
    int glyphIndex,
    int bitmapSize) {
  auto shape = makeShape(getGlyphShape(glyphIndex));
  if (!shape.validate()) {
    MultiLogger::get()->error(
        "Error: problem with shape from glyph index {}.", glyphIndex);
  }

  msdfgen::edgeColoringSimple(shape, 2.8);
  shape.normalize();
  Rect<double> shapeBounds{};
  shape.bounds(
      shapeBounds.xmin, shapeBounds.ymin, shapeBounds.xmax, shapeBounds.ymax);

  auto output = msdfgen::Bitmap<msdfgen::FloatRGB>(bitmapSize, bitmapSize);
  auto bitmapSizeShapeUnits = bitmapSize / scaleFactor;
  // msdfgen::Vector2 scaleToOutput{scaleFactor, scaleFactor};
  msdfgen::Vector2 translateShapeUnits{
      calcTranslation<double>(
          shapeBounds.xmin, shapeBounds.xmax, 0, bitmapSizeShapeUnits),
      calcTranslation<double>(
          shapeBounds.ymin, shapeBounds.ymax, 0, bitmapSizeShapeUnits)};
  auto rangeShapeUnits = pixelRange / scaleFactor;
  msdfgen::generateMSDF(
      output,
      shape,
      rangeShapeUnits,
      {scaleFactor, scaleFactor},
      translateShapeUnits);

  auto renderedGlyphBounds =
      flipY(getGlyphRenderBounds(shapeBounds, scaleFactor));
  auto paddedRenderedBounds = padRect(renderedGlyphBounds, padding);

  auto translateRenderUnits = translateShapeUnits * scaleFactor;
  auto transformedRenderBounds = getTransformedRenderBounds(
      renderedGlyphBounds,
      static_cast<float>(translateRenderUnits.x),
      static_cast<float>(translateRenderUnits.y));
  auto paddedTransformedRenderBounds =
      padRect(transformedRenderBounds, padding);
  auto uv = getUV(paddedTransformedRenderBounds, bitmapSize);

  return std::make_unique<MSDFGlyph>(MSDFGlyph{
      std::move(output), std::move(paddedRenderedBounds), std::move(uv)});
}

template <>
MSDFGlyphMap Font<>::getGlyphMap(int bitmapSize) {
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
    result[glyphIndex] = getMSDFGlyph(glyphIndex, bitmapSize);
  }

  return result;
}

template <>
Font<>::Font(fs::path fontPath, int bitmapSize, int padding)
    : bitmapSize(bitmapSize), padding(padding) {
  if (auto loadResult = vka::loadBinaryFile({fontPath})) {
    fontBytes = std::move(loadResult.value());
    stbtt_InitFont(&fontInfo, fontBytes.data(), 0);
  }
  originalPixelHeight = bitmapSize - (padding * 2);
  scaleFactor = vectorToRenderRatio(originalPixelHeight);
  glyphMap = getGlyphMap(bitmapSize);
  uint32_t arrayIndex{};
  for (const auto& [glyphIndex, bitmap] : glyphMap) {
    bitmap->arrayIndex = arrayIndex;
    ++arrayIndex;
  }
}
}  // namespace Text