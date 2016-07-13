#include <L/src/L.h>
#include <L/src/interface/stb.h>
#include <L/src/interface/obj.h>

using namespace L;

int main(int argc,const char* argv[]) {
  new STB();
  new OBJ();
  TypeInit();
  Window::openFullscreen("Test",Window::nocursor|Window::capturecursor);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  Engine::addUpdate<SpriteAnimator>();
  Engine::addUpdate<ScriptComponent>();
  Engine::addUpdate<RigidBody>();
  Engine::addUpdate<Collider>();
  Engine::addRender<Sprite>();
  Engine::addRender<StaticMesh>();
  Engine::addRender<Collider>();
  Engine::addEvent<ScriptComponent>();
  {
    ScriptComponent::init();
    FileStream file("startup.ls","rb");
    if(file){
      Script::Context startupContext;
      startupContext.read(file);
    } else out << "Couldn't open file startup.ls\n";
  }
  while(Window::loop())
    Engine::update();
  return 0;
}
