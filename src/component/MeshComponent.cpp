#include "MeshComponent.h"

#include "../rendering/GL.h"
#include "../rendering/shader_lib.h"

using namespace L;

Map<Symbol, Var> MeshComponent::pack() const {
  Map<Symbol, Var> data;
  data["mesh"] = _mesh;
  data["material"] = _material;
  data["scale"] = _scale;
  return data;
}
void MeshComponent::unpack(const Map<Symbol, Var>& data) {
  unpack_item(data, "mesh", _mesh);
  unpack_item(data, "material", _material);
  unpack_item(data, "scale", _scale);
}
void MeshComponent::script_registration() {
  L_COMPONENT_BIND(MeshComponent, "mesh");
  L_COMPONENT_METHOD(MeshComponent, "mesh", 1, mesh((const char*)c.local(0).get<String>()));
  L_COMPONENT_RETURN_METHOD(MeshComponent, "material", 0, material());
  L_COMPONENT_METHOD(MeshComponent, "scale", 1, scale(c.local(0).get<float>()));
}

void MeshComponent::late_update() {
  if(_mesh) {
    Interval3f world_bounds(_transform->position());
    const Interval3f& model_bounds(_mesh->bounds());
    for(uint32_t i(0); i<8; i++)
      world_bounds.add(_transform->toAbsolute(Vector3f(
        i&1 ? model_bounds.min().x() : model_bounds.max().x(),
        i&2 ? model_bounds.min().y() : model_bounds.max().y(),
        i&4 ? model_bounds.min().z() : model_bounds.max().z()
      )));
    _cull_volume.update_bounds(world_bounds);
  }
}

void MeshComponent::render(const Camera& c) {
  if(_cull_volume.visible() && _mesh && _material.valid()) {
    _material.use(SQTToMat(_transform->rotation(), _transform->position(), _scale));
    _mesh->draw();
  }
}
