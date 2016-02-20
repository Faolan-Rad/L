#include "Entity.h"

using namespace L;

Entity::Entity(const Entity* other) {
  for(int i(0); i<other->_components.size(); i++) {
    KeyValue<const TypeDescription*,Component*> p(other->_components[i]);
    p.value() = p.value()->clone();
    p.value()->entity(this);
    _components.push(p);
    p.value()->start();
  }
}
Entity::~Entity() {
  for(int i(0); i<_components.size(); i++)
    _components[i].value()->destruct();
}