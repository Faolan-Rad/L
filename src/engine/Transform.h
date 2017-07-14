#pragma once

#include "Component.h"
#include "../math/geometry.h"
#include "../math/Matrix.h"
#include "../math/Quaternion.h"
#include "../math/Vector.h"

namespace L {
  class Transform : public Component {
    L_COMPONENT(Transform)
  private:
    Vector3f _translation;
    Quatf _rotation;
  public:
    inline Transform() : _translation(0, 0, 0) {}

    virtual Map<Symbol, Var> pack() const override;
    virtual void unpack(const Map<Symbol, Var>&) override;

    inline Vector3f absolutePosition() const { return _translation; }
    inline Vector3f toAbsolute(const Vector3f& v) const { return absolutePosition()+_rotation.rotate(v); }
    inline Vector3f fromAbsolute(const Vector3f& v) const { return _rotation.inverse().rotate(v-absolutePosition()); }
    inline Quatf absoluteRotation() const { return _rotation; }
    inline void rotate(const Quatf& q) { _rotation = _rotation * q; }
    inline void rotate(const Vector3f& v, float d) { rotate(Quatf(v, d)); }
    inline void rotateAbsolute(const Quatf& q) { _rotation = q * _rotation; } // Wrong
    inline void rotateAbsolute(const Vector3f& v, float d) { rotateAbsolute(Quatf(v, d)); }
    inline void phiLook(float d) { rotate(Vector3f(1, 0, 0), d); }
    inline void thetaLook(float d) { rotate(Vector3f(0, 0, 1), d); }
    inline void move(const Vector3f& d) { _translation += _rotation.rotate(d); }
    inline void moveAbsolute(const Vector3f& v) { _translation += v; }

    inline void translation(const Vector3f& t) { _translation = t; }
    inline const Vector3f& translation() const { return _translation; }
    inline void rotation(const Quatf& r) { _rotation = r; }
    inline const Quatf& rotation() const { return _rotation; }
    inline Vector3f right() const { return absoluteRotation().rotate(Vector3f(1, 0, 0)); }
    inline Vector3f forward() const { return absoluteRotation().rotate(Vector3f(0, 1, 0)); }
    inline Vector3f up() const { return absoluteRotation().rotate(Vector3f(0, 0, 1)); }
    inline Matrix44f matrix() const { return SQTToMat(absoluteRotation(), absolutePosition()); }
  };
}
