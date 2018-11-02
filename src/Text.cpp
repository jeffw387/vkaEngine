#include "Text.hpp"

namespace FreeType {
Glyph::Glyph(FT_Glyph glyph) : glyph(glyph) {};

Glyph::~Glyph() {
  FT_Done_Glyph(glyph);
}

gsl::span<unsigned char> Glyph::render() {
  auto newGlyph = glyph;
  FT_Glyph_To_Bitmap(&newGlyph, FT_RENDER_MODE_NORMAL, nullptr, 0);
  bitmapGlyph = (FT_BitmapGlyph)newGlyph;
}
}