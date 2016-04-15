#include "Image.h"

#include "../gl/GL.h"

using namespace L;
using namespace GUI;

GUI::Image::Image() : Solid(), _dimensions(0,0) {}
GUI::Image::Image(const Bitmap& bmp) : Solid() {
  bitmap(bmp);
}
void GUI::Image::bitmap(const Bitmap& bmp) {
  _texture.load(bmp);
  _dimensions.x() = bmp.width();
  _dimensions.y() = bmp.height();
  dimensionsChanged(_dimensions);
}

Vector2i GUI::Image::gDimensions() {
  return _dimensions;
}
void GUI::Image::draw(GL::Program& program) {
  if(!_texture.empty() && !clip.empty()) {
    program.uniform("texture",_texture);
    glColor4ub(255,255,255,255);
    glBegin(GL_QUADS);
    glTexCoord2f((clip.min().x()>pos.x()) ? (float)(clip.min().x()-pos.x())/_dimensions.x() : 0.f,
                 (clip.min().y()>pos.y()) ? (float)(clip.min().y()-pos.y())/_dimensions.y() : 0.f);
    glVertex2i(clip.min().x(),clip.min().y());
    glTexCoord2f((clip.min().x()>pos.x()) ? (float)(clip.min().x()-pos.x())/_dimensions.x() : 0.f,
                 (clip.max().y()<pos.y()+_dimensions.y()) ? (float)(clip.max().y()-pos.y())/_dimensions.y() : 1.f);
    glVertex2i(clip.min().x(),clip.max().y());
    glTexCoord2f((clip.max().x()<pos.x()+_dimensions.x()) ? (float)(clip.max().x()-pos.x())/_dimensions.x() : 1.f,
                 (clip.max().y()<pos.y()+_dimensions.y()) ? (float)(clip.max().y()-pos.y())/_dimensions.y() : 1.f);
    glVertex2i(clip.max().x(),clip.max().y());
    glTexCoord2f((clip.max().x()<pos.x()+_dimensions.x()) ? (float)(clip.max().x()-pos.x())/_dimensions.x() : 1.f,
                 (clip.min().y()>pos.y()) ? (float)(clip.min().y()-pos.y())/_dimensions.y() : 0.f);
    glVertex2i(clip.max().x(),clip.min().y());
    glEnd();
    _texture.unbind();
  }
}