#include "ft2build.h"
#include FT_FREETYPE_H
#include <memory>
#include <string>
#include <optional>

namespace FreeType {

class GlyphSlot {
public:

private:
};

class Size {
public:
private:
};

class Face {
public:
  Face(FT_Library library, std::string fontPath, FT_Long faceIndex);
  GlyphSlot* loadGlyph(FT_ULong character);  
private:
  FT_Face* face;
};

class Font {
public:
Font(FT_Library library);
std::unique_ptr<Face> createFace(FT_Long face_index);
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
}