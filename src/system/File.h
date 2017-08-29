#pragma once

#include "../text/String.h"
#include "../time/Date.h"
#include "System.h"

namespace L {
  class File {
  protected:
    String _path;
  public:
    inline File(const char* path) : _path(System::formatPath(path)) {}
    inline bool operator<(const File& other) const { return _path < other._path; }

    inline String name() const { return System::pathFile(_path); }
    inline String dir() const { return System::pathDirectory(_path); }
    inline String ext() const { return &_path[_path.findLast('.')+1]; }
    inline const String& path() const { return _path; }

    bool exists() const;
    void make() const;
    bool mtime(Date&) const;
  };
}
