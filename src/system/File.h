#ifndef DEF_L_System_File
#define DEF_L_System_File

#include "../String.h"

namespace L {
  class File {
    protected:
      String _path;
    public:
      File(const char* path);

      bool operator<(const File&) const;

      String name() const;
      String dir() const;
      inline const String& path() const {return _path;}

      bool exists() const;
      void make() const;
  };
}

#endif


