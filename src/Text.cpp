#define STB_RECT_PACK_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include "Text.hpp"

namespace Text {
using namespace ranges;

template <>
float Font<>::msdfToRenderRatio(int fontPixelHeight) {
  return static_cast<float>(fontPixelHeight) /
         static_cast<float>(originalPixelHeight);
}

template <>
float Font<>::vectorToRenderRatio(int fontPixelHeight) {
  return stbtt_ScaleForPixelHeight(&fontInfo, fontPixelHeight);
}

template <>
float Font<>::getAdvance(int glyphIndex, int fontPixelHeight) {
  int adv{};
  int lsb{};
  stbtt_GetGlyphHMetrics(&fontInfo, glyphIndex, &adv, &lsb);
  return vectorToRenderRatio(fontPixelHeight) * static_cast<float>(adv + lsb);
}

template <>
float Font<>::getKerning(
    int glyphIndex1,
    int glyphIndex2,
    int fontPixelHeight) {
  return vectorToRenderRatio(fontPixelHeight) *
         static_cast<float>(
             stbtt_GetGlyphKernAdvance(&fontInfo, glyphIndex1, glyphIndex2));
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
        std::vector<Vertex>{{{pos.left, pos.top}, {uv.left, uv.top}},
                            {{pos.left, pos.bottom}, {uv.left, uv.bottom}},
                            {{pos.right, pos.bottom}, {uv.right, uv.bottom}},
                            {{pos.right, pos.top}, {uv.right, uv.top}}});
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

  auto left_trans = static_cast<float>(left + translate.x) * scaleFactor;
  auto top_trans = static_cast<float>(-(top + translate.y)) * scaleFactor;
  auto right_trans = static_cast<float>(right + translate.x) * scaleFactor;
  auto bottom_trans = static_cast<float>(-(bottom + translate.y)) * scaleFactor;
  return std::make_unique<MSDFGlyph>(
      MSDFGlyph{std::move(output),
                {left_trans, top_trans, right_trans, bottom_trans},
                {left_trans / bitmapSize,
                 top_trans / bitmapSize,
                 right_trans / bitmapSize,
                 bottom_trans / bitmapSize}});
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
  originalPixelHeight = msdfSize - (padding * 2);
  auto scaleFactor = vectorToRenderRatio(originalPixelHeight);
  glyphMap = getGlyphMap(msdfSize, scaleFactor);
  uint32_t arrayIndex{};
  for (const auto& [glyphIndex, bitmap] : glyphMap) {
    bitmap->arrayIndex = arrayIndex;
    ++arrayIndex;
  }
}
}  // namespace Text