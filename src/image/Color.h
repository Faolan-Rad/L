#ifndef DEF_L_Image_Color
#define DEF_L_Image_Color

#include "../math/math.h"
#include "../math/Vector.h"
#include "../String.h"

namespace L {
  class Color  : public Vector4b {
    public:
      inline Color() : Vector4b(0,0,0,0) {}
      inline Color(const Vector4b& v) : Vector4b(v) {}
      inline Color(byte gs) : Vector4b(gs,gs,gs,0xff) {}
      inline Color(byte r, byte g, byte b, byte a = 0xFF) : Vector4b(b,g,r,a) {}
      Color(String);

      inline byte r() const {return _c[2];}
      inline byte g() const {return _c[1];}
      inline byte b() const {return _c[0];}
      inline byte a() const {return _c[3];}
      inline uint rgb() const {return r()|g()<<8|b()<<16;}
      inline uint rgba() const {return r()<<8|g()<<16|b()<<24|a();}
      inline byte& r() {return _c[2];}
      inline byte& g() {return _c[1];}
      inline byte& b() {return _c[0];}
      inline byte& a() {return _c[3];}

      static Color from(float r, float g, float b, float a = 1);
      static Color lerp(Color, Color, float w);
      static const Color black, blue, cyan, green, grey, lightgrey, magenta, red, white, yellow;
  };
  inline Stream& operator>>(Stream &s, Color& v) {
    v = Color(s.word());
    return s;
  }
  inline Stream& operator<<(Stream &s, const Color& v) {
    return s << '#' << numberToString<16>(v.rgba(),8);
  }
}

#endif




