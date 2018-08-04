#pragma once

#include "Component.h"
#include "Transform.h"
#include "../engine/CullVolume.h"
#include "../engine/Resource.h"
#include "../rendering/Pipeline.h"
#include "../rendering/DescriptorSet.h"
#include "../rendering/Framebuffer.h"

namespace L {
  class LightComponent : public Component {
    L_COMPONENT(LightComponent)
      L_COMPONENT_HAS_LATE_UPDATE(LightComponent)
  protected:
    static Resource<Pipeline> _pipeline;
    Transform* _transform;
    CullVolume _cull_volume;
    struct Values {
      Vector4f dir, color;
      float intensity, radius, inner_angle, outer_angle;
      int type;
    } _values;
    Vector3f _relative_dir;
  public:
    inline LightComponent() { point(Color::white); }

    virtual void update_components() override { _transform = entity()->requireComponent<Transform>(); }
    virtual Map<Symbol, Var> pack() const override;
    virtual void unpack(const Map<Symbol, Var>&) override;
    static void script_registration();

    void late_update();

    void directional(const Color& color, const Vector3f& direction, float intensity = 1.f);
    void point(const Color& color, float intensity = 1.f, float radius = 1.f);
    void spot(const Color& color, const Vector3f& direction, float intensity = 1.f, float radius = 1.f, float inner_angle = .5f, float outer_angle = 0.f);

    void render(VkCommandBuffer cmd_buffer, const Framebuffer& framebuffer);
    inline static void pipeline(const char* path) { _pipeline = path; }
    inline static Resource<Pipeline>& pipeline() { return _pipeline; }
  };
}
