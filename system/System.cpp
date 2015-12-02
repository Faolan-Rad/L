#include "System.h"

using namespace L;
using namespace System;

#include "Directory.h"
#include "../Exception.h"

String System::callGet(const char* cmd) {
  String wtr = "";
  FILE* pipe = popen(cmd,"r");
  if(!pipe) throw Exception("Couldn't open pipe in System");
  else {
    char buffer[128];
    while(!feof(pipe)) {
      if(fgets(buffer, 128, pipe) != NULL)
        wtr += buffer;
    }
    pclose(pipe);
  }
  return wtr;
}
int System::call(const char* cmd) {
  return system(cmd);
}
void System::sleep(int milliseconds) {
  sleep(Time(0,milliseconds));
}
void System::sleep(const Time& t) {
#if defined L_WINDOWS
  Sleep(t.milliseconds());
#elif defined L_UNIX
  usleep(t.microseconds());
#endif
}
void System::beep(uint frequency, uint milliseconds) {
#if defined L_WINDOWS
  ::Beep(frequency,milliseconds);
#elif defined L_UNIX
  throw Exception("Cannot beep under UNIX.");
#endif
}
ullong System::ticks() {
#if defined _MSC_VER // MSVC
  return __rdtsc();
#else // GCC
  ullong wtr;
  __asm__ __volatile__
  (
    "cpuid \n"
    "rdtsc \n"
    "leal %0, %%ecx \n"
    "movl %%eax, (%%ecx) \n"
    "movl %%edx, 4(%%ecx)" :: "m"(wtr) : "eax", "ebx", "ecx", "edx"
  );
  return wtr;
#endif
}
void System::toClipboard(const String& data) {
#if defined L_WINDOWS
  if(OpenClipboard(NULL) && EmptyClipboard()) {
    HGLOBAL tmp = GlobalAlloc(0,data.size()+1); // Allocate global memory
    GlobalLock(tmp); // Lock it
    strcpy((char*)tmp,data); // Copy data
    GlobalUnlock(tmp); // Unlock it
    SetClipboardData(CF_TEXT,tmp);
    CloseClipboard();
  } else throw Exception("Couldn't open clipboard.");
#endif
}
String System::fromClipboard() {
#if defined L_WINDOWS
  HGLOBAL tmp;
  if(OpenClipboard(NULL) && (tmp = GetClipboardData(CF_TEXT))) {
    String wtr((char*)tmp);
    CloseClipboard();
    return wtr;
  }
#endif
  throw Exception("Cannot get clipboard data.");
}
String System::env(const String& name) {
  return getenv(name);
}
Vector2i System::screenSize() {
#if defined L_WINDOWS
  return Vector2i(GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN));
#endif
}
void System::consoleCursorPosition(ushort x,ushort y) {
#if defined L_WINDOWS
  HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
  COORD position = {(short)x,(short)y};
  SetConsoleCursorPosition(hStdout,position);
#elif defined L_UNIX
  move(y,x);
  refresh();
#endif
}
void System::clearConsole() {
#if defined L_WINDOWS
  call("cls");
#elif defined L_UNIX
  call("clear");
#endif
}
void System::closeConsole() {
#if defined L_WINDOWS
  FreeConsole();
#elif defined L_UNIX
  throw Exception("Closing the console under UNIX is impossible.");
#endif
}

String System::formatPath(String path) {
#if defined L_WINDOWS
  path.replaceAll("/","\\");
#endif
#if defined L_WINDOWS
  if(path != "\\" && (path.size()<2 || path[1]!=':'))
#elif defined L_UNIX
  if(!path.size() || path[0]!='/')
#endif
    path = Directory::current.gPath()+path;
  return path;
}
String System::pathDirectory(const String& path) {
  return path.substr(0,1+path.findLast(slash));
}
String System::pathFile(const String& path) {
  return path.substr(1+path.findLast(slash));
}

