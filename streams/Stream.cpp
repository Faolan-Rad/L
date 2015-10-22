#include "Stream.h"

using namespace L;

const char* Stream::line() {
  static char buffer[4096];
  char* w(buffer);
  nospace();
  while((*w++ = get())!='\n' && !eof()) {} // End of line or file
  while(*(w-1)=='\n' || *(w-1)=='\r') w--; // Don't take \n into account
  *w = '\0'; // Null-end string
  return buffer;
}
const char* Stream::word() {
  static char buffer[1024];
  char* w(buffer);
  nospace();
  while(!isspace(*w = get()) && !eof()) w++; // End of line or file
  *w = '\0'; // Null-end string
  return buffer;
}
const char* Stream::bufferize(size_t* size) {
  static char buffer[4096];
  char* w(buffer);
  while(!eof() && w<buffer+4096) {*w++ = get();} // End of line or file
  if(w==buffer) return 0;
  else {
    *size = w-buffer;
    return buffer;
  }
}
