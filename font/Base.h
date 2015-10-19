#ifndef DEF_L_Font_Base
#define DEF_L_Font_Base

#include "../containers/Map.h"
#include "../image.h"
#include "../geometry/Vector.h"

namespace L {
  namespace Font {
    typedef struct {
      L::Bitmap bmp;
      Vector2i origin;
      size_t advance;
    } Glyph;
    class Base {
      protected:
        Map<size_t,Glyph> _glyphs;
      public:
        virtual ~Base() {}
        Glyph& glyph(size_t utf32);
        virtual L::Bitmap render(const String&);

        virtual size_t lineHeight() = 0;
        virtual size_t ascender() = 0;
        virtual size_t descender() = 0;
        virtual Glyph loadGlyph(size_t utf32) = 0;
    };
  }
}

#endif

