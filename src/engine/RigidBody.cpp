#include "RigidBody.h"

using namespace L;

void RigidBody::start() {
  _transform = entity()->requireComponent<Transform>();
  _invMass = 1.f;
  _velocity = Vector3f(0.f,0.f,0.f);
  _rotVel = Vector3f(0.f,0.f,0.f);
  _invInertiaTensor = Matrix33f::identity();
}
void RigidBody::update() {
  _transform->moveAbsolute(_velocity*Engine::deltaSeconds());
  float rotLength(_rotVel.length());
  if(rotLength>.001f)
    _transform->rotateAbsolute(_rotVel*(1.f/rotLength),rotLength*Engine::deltaSeconds());
  _velocity += Engine::deltaSeconds()*Vector3f(0.f,0.f,-9.8f);
}

Vector3f RigidBody::velocityAt(const Vector3f& offset) const{
  return _rotVel.cross(offset)+_velocity;
}

float RigidBody::deltaVelocity(const Vector3f& offset,const Vector3f& normal) const{
  Vector3f torquePerUnitImpulse(offset.cross(normal));
  Vector3f rotationPerUnitImpulse(_invInertiaTensor*torquePerUnitImpulse);
  Vector3f velocityPerUnitImpulse(rotationPerUnitImpulse.cross(offset));
  float angularComponent(velocityPerUnitImpulse.dot(normal));
  return angularComponent+_invMass;
}
void RigidBody::applyImpulse(const Vector3f& impulse,const Vector3f& offset){
  addForce(impulse);
  addTorque(offset.cross(impulse));
}
float restitution(.3f);
void RigidBody::collision(RigidBody* a,RigidBody* b,const Vector3f& impact,const Vector3f& normal) {
  Vector3f
    arel(impact-a->_transform->absolutePosition()),
    brel((b) ? impact-b->_transform->absolutePosition() : 0),
    av(a->velocityAt(arel)),
    bv((b) ? b->velocityAt(brel) : 0);

  float contactVelocity((av-bv).dot(normal));
  float desiredDeltaVelocity(-contactVelocity*(1.f+restitution));
  float deltaVelocity(a->deltaVelocity(arel,normal));
  if(b) deltaVelocity += b->deltaVelocity(brel,normal);
  Vector3f impulse(normal*(desiredDeltaVelocity/deltaVelocity));
  if(contactVelocity<0.f){
    a->applyImpulse(impulse,arel);
    if(b) b->applyImpulse(-impulse,brel);
  }
}