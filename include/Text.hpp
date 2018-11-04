#include "ft2build.h"
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include <memory>
#include <vector>
#include <string>
#include <optional>
#include <gsl-lite.hpp>
#include "Logger.hpp"
#include <range/v3/all.hpp>
using namespace ranges;

// Flattens a range of ranges by iterating the inner
// ranges in round-robin fashion.
template <class Rngs>
class interleave_view : public view_facade<interleave_view<Rngs>> {
  friend range_access;
  std::vector<range_value_type_t<Rngs>> rngs_;
  struct cursor;
  cursor begin_cursor() {
    return {0, &rngs_, view::transform(rngs_, ranges::begin)};
  }

public:
  interleave_view() = default;
  explicit interleave_view(Rngs rngs) : rngs_(std::move(rngs)) {}
};

template <class Rngs>
struct interleave_view<Rngs>::cursor {
  std::size_t n_;
  std::vector<range_value_type_t<Rngs>> *rngs_;
  std::vector<iterator_t<range_value_type_t<Rngs>>> its_;
  decltype(auto) read() const { return *its_[n_]; }
  void next() {
    if (0 == ((++n_) %= its_.size())) for_each(its_, [](auto &it) { ++it; });
  }
  bool equal(default_sentinel) const {
    return n_ == 0 &&
           its_.end() !=
               mismatch(
                   its_, *rngs_, std::not_equal_to<>(), ident(), ranges::end)
                   .in1();
  }
  CONCEPT_REQUIRES(ForwardRange<range_value_type_t<Rngs>>())
  bool equal(cursor const &that) const {
    return n_ == that.n_ && its_ == that.its_;
  }
};

// In:  Range<Range<T>>
// Out: Range<T>, flattened by walking the ranges
//                round-robin fashion.
inline auto interleave() {
  return make_pipeable([](auto &&rngs) {
    using Rngs = decltype(rngs);
    return interleave_view<view::all_t<Rngs>>(
        view::all(std::forward<Rngs>(rngs)));
  });
}

// In:  Range<Range<T>>
// Out: Range<Range<T>>, transposing the rows and columns.
inline auto transpose() {
  return make_pipeable([](auto &&rngs) {
    using Rngs = decltype(rngs);
    CONCEPT_ASSERT(ForwardRange<Rngs>());
    return std::forward<Rngs>(rngs) | interleave() |
           view::chunk(static_cast<std::size_t>(distance(rngs)));
  });
}


struct BBox {
  int xmin;
  int ymin;
  int xmax;
  int ymax;
};

struct Dimensions {
  int width;
  int height;
};

struct Tile {
  gsl::span<unsigned char> tileData;
  int pitch;
  int rowcount;

  gsl::span<unsigned char> getRow(size_t rowIndex);
};

struct Row {
  std::vector<Tile> tiles;
};

struct Tileset {
  std::vector<Row> rows;
};

inline Tileset makeTileset(std::vector<Tile> tiles, size_t maxTilesetWidth) {
  Tileset result;
  
}

class Glyph {
public:
  Glyph(FT_Glyph glyph);
  ~Glyph();
  gsl::span<unsigned char> render();
  BBox getBoundingBox();
  Dimensions getDimensions();

private:
  FT_Glyph glyph;
  FT_BitmapGlyph bitmapGlyph;
  bool rendered;
};

class Face {
public:
  Face(FT_Library library, std::string fontPath, FT_Long faceIndex);
  ~Face();
  std::unique_ptr<Glyph> loadChar(FT_ULong character);
  std::unique_ptr<Glyph> loadGlyph(FT_UInt glyphIndex);
  std::vector<FT_ULong> getCharacters();
  std::vector<std::unique_ptr<Glyph>> getGlyphs();
  void setSize(uint8_t fontSize, FT_UInt dpi);

private:
  FT_Face face;

  std::unique_ptr<Glyph> getGlyph();
};

class Font {
public:
  Font(FT_Library library, std::string fontPath);
  std::unique_ptr<Face> createFace(FT_Long faceIndex);

private:
  FT_Library library;
  std::string fontPath;
};

class Library {
public:
  Library();
  ~Library();
  Library(const Library&) = delete;
  Library& operator=(const Library&) = delete;

  std::unique_ptr<Font> loadFont(std::string fontPath);

private:
  FT_Library library;
};

inline void testLoad() {
  // std::string fontPath = "content/fonts/arial.ttf";
  // Library library;
  // auto font = library.loadFont(fontPath);
  // auto face = font->createFace(0);
}

}  // namespace FreeType