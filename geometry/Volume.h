#ifndef DEF_L_Volume
#define DEF_L_Volume

#include "../Exception.h"
#include "Point.h"

namespace L{
    static const Exception vex("No volume can exist in a one or two dimensional space.");
    template <int di,class T>
    class Volume{
        protected:
            Point<di,T> a, b, c, d;
        public:
            Volume(){
                if(di<3) throw sex;
            }
            Volume(const Point<di,T>& a,
                   const Point<di,T>& b,
                   const Point<di,T>& c,
                   const Point<di,T>& d)
                   : a(a), b(b), c(c), d(d){
                if(di<3) throw sex;
            }

            bool contains(const Point<di,T>& p){
                throw Exception("This feature hasn't been done yet.");
            }
            bool empty() const{
                return ((a == b) && (b == c) && (c == d));
            }

            inline const Point<di,T>& gA() const{return a;}
            inline const Point<di,T>& gB() const{return b;}
            inline const Point<di,T>& gC() const{return c;}
            inline const Point<di,T>& gD() const{return d;}
    };
}

#endif
