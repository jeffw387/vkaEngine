#pragma once
#include <glm/glm.hpp>
#include "Force.hpp"

namespace Physics {
class Particle {
public:
  glm::vec3 position = {};
  glm::vec3 velocity = {};
  glm::vec3 acceleration = {};
  glm::vec3 forceAccum = {};
  float damping = {};
  float inverseMass = {};

  void setMass(float mass);
};

class ParticleForceGenerator {
  virtual void updateForces(Particle* particle, float duration) = 0;
};

inline Particle integrate(Particle input, float timeScale) {
  input.position += input.velocity * timeScale;
  input.acceleration += input.forceAccum * input.inverseMass;
  input.velocity += input.acceleration * timeScale;
  input.velocity *= input.damping;
  input.forceAccum = {};
  return input;
}

}  // namespace Physics