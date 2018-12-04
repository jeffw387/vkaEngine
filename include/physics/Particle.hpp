#pragma once
#include <vector>
#include <glm/glm.hpp>

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
public:
  virtual void updateForce(Particle* particle, float duration) = 0;
};

class ParticleForceRegistry {
protected:
  struct ParticleForceRegistration {
    Particle* particle;
    ParticleForceGenerator* generator;
  };

  using Registry = std::vector<ParticleForceRegistration>;
  Registry registry;

public:
  void add(Particle* particle, ParticleForceGenerator* generator);
  void remove(Particle* particle, ParticleForceGenerator* generator);
  void clear();
  void updateForces(float duration);
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