#ifndef DEF_L_XML
#define DEF_L_XML

#include "../stl/Map.h"
#include "../stl/String.h"
#include "../stl/Vector.h"
#include "../system/File.h"

namespace L{
    class XML : public Serializable{
        public:
            String name;
            Vector<XML> children;
            Map<String,String> attributes;
            bool text;

            XML();
            XML(const File&);
            XML(std::istream&);

            void write(std::ostream&) const;
            void read(std::istream&);
    };
}

#endif



