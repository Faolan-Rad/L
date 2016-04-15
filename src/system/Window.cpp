#include "Window.h"

#include "../systems.h"

#include <cstring>
#include "../gl/GL.h"
#include "../bytes/encoding.h"
#include "System.h"
#include "../containers/StaticRing.h"

using namespace L;
using L::Window;

bool buttonstate[Window::Event::LAST] = {false};
StaticRing<512,Window::Event> _events;
Vector2i _mousePos;
int _width, _height, _flags;

Window::Event::Event() {
  memset(this,0,sizeof(*this));
}

#if defined L_WINDOWS
HWND hWND;
HDC hDC;
HGLRC hRC;
HBITMAP hBITMAP = 0;
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  Window::Event e;
  switch(uMsg) {
    case WM_CREATE:
      return 0;
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      hWND = 0;
      return 0;
      break;
    case WM_KEYDOWN:
    case WM_KEYUP: // Key pressed
      e.type = (uMsg==WM_KEYDOWN) ? Window::Event::BUTTONDOWN : Window::Event::BUTTONUP;
      switch(wParam) {
#define MAP(a,b) case a: e.button = Window::Event::b; break;
          MAP(VK_BACK,BACKSPACE)
          MAP(VK_TAB,TAB)
          MAP(VK_RETURN,ENTER)
          MAP(VK_SHIFT,SHIFT)
          MAP(VK_CONTROL,CTRL)
          MAP(VK_MENU,ALT)
          MAP(VK_PAUSE,PAUSE)
          MAP(VK_CAPITAL,CAPS)
          MAP(VK_ESCAPE,ESCAPE)
          MAP(VK_SPACE,SPACE)
          MAP(VK_LEFT,LEFT) MAP(VK_RIGHT,RIGHT) MAP(VK_UP,UP) MAP(VK_DOWN,DOWN)
          MAP(VK_NUMLOCK,NUMLOCK)
          MAP(VK_NUMPAD1,NUM1) MAP(VK_NUMPAD2,NUM2) MAP(VK_NUMPAD3,NUM3)
          MAP(VK_NUMPAD4,NUM4) MAP(VK_NUMPAD5,NUM5) MAP(VK_NUMPAD6,NUM6)
          MAP(VK_NUMPAD7,NUM7) MAP(VK_NUMPAD8,NUM8) MAP(VK_NUMPAD9,NUM9)
          MAP(VK_NUMPAD0,NUM0)
          MAP(VK_F1,F1) MAP(VK_F2,F2) MAP(VK_F3,F3) MAP(VK_F4,F4)
          MAP(VK_F5,F5) MAP(VK_F6,F6) MAP(VK_F7,F7) MAP(VK_F8,F8)
          MAP(VK_F9,F9) MAP(VK_F10,F10) MAP(VK_F11,F11) MAP(VK_F12,F12)
#undef MAP
        default:
          if((wParam>='0' && wParam<='9') || (wParam>='A' && wParam<='Z'))
            e.button = (Window::Event::Button)wParam;
          else return 0;
      }
      break;
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
      if(uMsg==WM_MOUSEMOVE)
        e.type = Window::Event::MOUSEMOVE;
      else {
        if(uMsg==WM_LBUTTONDOWN || uMsg==WM_RBUTTONDOWN || uMsg==WM_MBUTTONDOWN)
          e.type = Window::Event::BUTTONDOWN;
        else e.type = Window::Event::BUTTONUP;
        switch(uMsg) {
          case WM_LBUTTONDOWN:
          case WM_LBUTTONUP:
            e.button = Window::Event::LBUTTON;
            break;
          case WM_RBUTTONDOWN:
          case WM_RBUTTONUP:
            e.button = Window::Event::RBUTTON;
            break;
          case WM_MBUTTONDOWN:
          case WM_MBUTTONUP:
            e.button = Window::Event::MBUTTON;
            break;
        }
      }
      e.x = GET_X_LPARAM(lParam);
      e.y = GET_Y_LPARAM(lParam);
      _mousePos = Vector2i(e.x,e.y);
      break;
    case WM_MOUSEWHEEL:
      e.type = Window::Event::MOUSEWHEEL;
      e.y = GET_WHEEL_DELTA_WPARAM(wParam);
      break;
    case WM_SIZE: // The size of the window has changed
      e.type = Window::Event::RESIZE;
      e.x = LOWORD(lParam);
      e.y = HIWORD(lParam);
      break;
    case WM_CHAR:
      if(wParam != '\b' && wParam != '\r') {
        e.type = Window::Event::TEXT;
        strcpy(e.text,UTF16toUTF8(wParam));
      } else return 0;
      break;
    case WM_SETCURSOR:
      SetCursor((_flags & Window::nocursor)?nullptr:LoadCursor(nullptr,IDC_ARROW));
      break;
    default:
      return DefWindowProc(hwnd, uMsg, wParam, lParam);
      break;
  }
  if(e.type==Window::Event::BUTTONDOWN)
    buttonstate[e.button] = true;
  else if(e.type==Window::Event::BUTTONUP)
    buttonstate[e.button] = false;
  _events.push(e);
  return 0;
}
#elif defined L_UNIX
bool                    winOpened(false);
Display                 *dpy;
::Window                root;
GLint                   att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
XVisualInfo             *vi;
Colormap                cmap;
XSetWindowAttributes    swa;
::Window                win;
GLXContext              glc;
XWindowAttributes       gwa;
void eventTranslate(const XEvent& xev) {
  Window::Event e;
  switch(xev.type) {
    case MotionNotify:
      e.type = Window::Event::MOUSEMOVE;
      e.x = xev.xmotion.x;
      e.y = xev.xmotion.y;
      break;
    case KeyPress:
      break;
    case ResizeRequest:
      e.type = Window::Event::RESIZE;
      e.x = xev.xresizerequest.width;
      e.y = xev.xresizerequest.height;
      XResizeWindow(dpy,win,e.x,e.y);
      break;
    case ClientMessage: // It's the close operation
      Window::close();
      break;
    default:
      return;
      break;
  }
  _events.push_back(e);
}
#endif

void Window::open(const char* title, int width, int height, int flags) {
  if(opened()) return;
  _width = width;
  _height = height;
  _flags = flags;
#if defined L_WINDOWS
  WNDCLASS wc;
  // Register window class
  wc.style = CS_OWNDC;
  wc.lpfnWndProc = MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = GetModuleHandle(nullptr);
  wc.hIcon = LoadIcon(nullptr,IDI_APPLICATION);
  wc.hCursor = (flags & nocursor)?nullptr:LoadCursor(nullptr,IDC_ARROW);
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = nullptr;
  wc.lpszClassName = "LWC";
  RegisterClass(&wc);
  DWORD wStyle = ((flags & borderless)?(WS_POPUP):(WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU))
                 | ((flags & resizable)?(WS_MAXIMIZEBOX):0)
                 | ((flags & resizable)?(WS_SIZEBOX):0)
                 | WS_VISIBLE;
  // Find out needed window size for wanted client area size
  RECT rect = {0,0,(int)width,(int)height};
  AdjustWindowRect(&rect,wStyle,false);
  width = rect.right-rect.left;
  height = rect.bottom-rect.top;
  // Create window
  hWND = CreateWindow("LWC",title,wStyle,   // Properties
                      CW_USEDEFAULT,CW_USEDEFAULT,width,height,
                      nullptr,nullptr,GetModuleHandle(nullptr),nullptr);
  hDC = GetDC(hWND); // Get the device context (DC)
  PIXELFORMATDESCRIPTOR pfd;
  ZeroMemory(&pfd,sizeof(pfd)); // Initialize pixel format descriptor
  pfd.nSize = sizeof(pfd);
  pfd.nVersion = 1;
  pfd.dwFlags = PFD_DRAW_TO_WINDOW | ((flags & opengl)?PFD_SUPPORT_OPENGL:0) | PFD_DOUBLEBUFFER;
  pfd.iPixelType = PFD_TYPE_RGBA;
  pfd.cColorBits = 24;
  pfd.cDepthBits = 16;
  SetPixelFormat(hDC,ChoosePixelFormat(hDC,&pfd),&pfd); // Set pixel format (according to descriptor) to device context
  if(flags & opengl) {
    hRC = wglCreateContext(hDC); // create and enable the render context (RC)
    wglMakeCurrent(hDC,hRC);
  }
#elif defined L_UNIX
  if((dpy = XOpenDisplay(nullptr)) == nullptr)
    L_ERROR("Cannot open X server display.");
  root = DefaultRootWindow(dpy);
  if((vi = glXChooseVisual(dpy, 0, att)) == nullptr)
    L_ERROR("No appropriate visual found for X server.");
  cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.event_mask = ExposureMask | KeyPressMask | VectorerMotionMask;
  win = XCreateWindow(dpy, root, 0, 0, width, height, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
  Atom delWindow = XInternAtom(dpy,"WM_DELETE_WINDOW",0); // This is for the window close operation
  XSetWMProtocols(dpy, win, &delWindow, 1);
  XMapWindow(dpy, win);
  XStoreName(dpy, win, title.c_str());
  glc = glXCreateContext(dpy, vi, nullptr, GL_TRUE);
  glXMakeCurrent(dpy, win, glc);
  winOpened = true;
#endif
  if(flags & opengl)
    GL::init();
}
void Window::openFullscreen(const char* title, int flags) {
  Vector2i screenSize(System::screenSize());
  open(title,screenSize.x(),screenSize.y(),borderless | flags);
}
void Window::close() {
  if(!opened()) return;
#if defined L_WINDOWS
  DestroyWindow(hWND);
  hWND = 0;
#elif defined L_UNIX
  glXMakeCurrent(dpy, None, nullptr);
  glXDestroyContext(dpy, glc);
  XDestroyWindow(dpy, win);
  XCloseDisplay(dpy);
  winOpened = false;
#endif
}
bool Window::opened() {
#if defined L_WINDOWS
  return hWND!=0;
#elif defined L_UNIX
  return (winOpened);
#endif
  return false;
}
bool Window::loop() {
#if defined L_WINDOWS
  MSG msg;
  while(opened() && PeekMessage(&msg,nullptr,0,0,PM_REMOVE)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
#elif defined L_UNIX
  XEvent xev;
  while(opened() && XPending(dpy)) {
    XNextEvent(dpy,&xev);
    eventTranslate(xev);
  }
#endif
  return opened();
}
bool Window::newEvent(Event& e) {
  if(_events.empty())
    return false;
  else {
    e = _events.top();
    _events.pop();
    return true;
  }
}
bool Window::isPressed(Event::Button button) {
  return buttonstate[button];
}

void Window::swapBuffers() {
  if(!opened()) return;
#if defined L_WINDOWS
  SwapBuffers(hDC);
#elif defined L_UNIX
  glXSwapBuffers(dpy, win);
#endif
}
void Window::draw(const Bitmap& bmp) {
  if(!opened()) return;
#if defined L_WINDOWS
  HBITMAP hbmp = CreateBitmap(bmp.width(),bmp.height(),1,32,&bmp(0,0)), htmp;
  HDC hMemDC = CreateCompatibleDC(hDC);
  htmp = (HBITMAP)SelectObject(hMemDC,hbmp);
  BitBlt(hDC,0,0,bmp.width(),bmp.height(),hMemDC,0,0,SRCCOPY);
  SelectObject(hMemDC, htmp);
  DeleteObject(hbmp);
  DeleteDC(hMemDC);
#elif defined L_UNIX
#endif
}

void Window::title(const char* str) {
  if(!opened()) return;
#if defined L_WINDOWS
  SetWindowText(hWND,str);
#elif defined L_UNIX
#endif
}
void Window::resize(int width, int height) {
  if(!opened()) return;
  _width = width;
  _height = height;
#if defined L_WINDOWS
  SetWindowPos(hWND,HWND_NOTOPMOST,0,0,width,height,SWP_NOMOVE|SWP_NOZORDER);
#elif defined L_UNIX
#endif
}
int Window::width() {
  return _width;
}
int Window::height() {
  return _height;
}
float Window::aspect() {
  return (float)_width/_height;
}
Vector2i Window::mousePosition() {
  return _mousePos;
}
Vector2f Window::normalizedMousePosition() {
  return Vector2f((2*(float)_mousePos.x()/_width)-1,-((2*(float)_mousePos.y()/_height)-1));
}
Vector2f Window::normalizedToPixels(const Vector2f& p) {
  return Vector2f(((p.x()+1)/2)*_width,((-p.y()+1)/2)*_height);
}