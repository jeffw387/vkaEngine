#pragma once
#include <vector>
#include <array>
#include <glm/glm.hpp>
#include <gsl-lite.hpp>

namespace Physics {
class Particle {
public:
  glm::vec3 position = {};
  glm::vec3 velocity = {};
  glm::vec3 constantAcceleration = {};
  glm::vec3 forceAccum = {};
  float damping = {};
  float inverseMass = {};

  void setMass(float mass);
  bool hasInfiniteMass() const;
  void integrate(float timeScale);
};

struct ParticleContact {
  std::array<Particle*, 2> particle = {};
  float restitution = {};
  glm::vec3 contactNormal = {};
  float penetration = {};

  void resolve(float duration);
  float calculateSeparatingVelocity() const;

private:
  float getTotalInverseMass() const;
  void resolveVelocity(float duration, float totalInverseMass);
  void resolveInterpenetration(float duration, float totalInverseMass);
};

struct ParticleContactResolver {
  int iterations = {};
  void resolveContacts(gsl::span<ParticleContact> contacts, float duration);

protected:
  int iterationsUsed = {};
};

class ParticleForceGenerator {
public:
  virtual void updateForce(Particle* particle, float duration) = 0;
};

class ParticleGravityGenerator : public ParticleForceGenerator {
public:
  glm::vec3 gravity = {};
  void updateForce(Particle* particle, float duration);
};

class ParticleDragGenerator : public ParticleForceGenerator {
public:
  float k1 = {};
  float k2 = {};
  void updateForce(Particle* particle, float duration);
};

class ParticleSpring : public ParticleForceGenerator {
public:
  Particle* other;
  float springConstant;
  float restLength;

  void updateForce(Particle* particle, float duration);
};

class ParticleAnchoredSpring : public ParticleForceGenerator {
public:
  glm::vec3 anchor;
  float springConstant;
  float restLength;

  void updateForce(Particle* particle, float duration);
};

class ParticleForceRegistry {
protected:
  struct ParticleForceRegistration {
    Particle* particle = {};
    ParticleForceGenerator* generator = {};
  };

  using Registry = std::vector<ParticleForceRegistration>;
  Registry registry = {};

public:
  void add(Particle* particle, ParticleForceGenerator* generator);
  void remove(Particle* particle, ParticleForceGenerator* generator);
  void clear();
  void updateForces(float duration);
};

inline Particle integrate(Particle input, float timeScale) {
  input.position += input.velocity * timeScale;
  input.constantAcceleration += input.forceAccum * input.inverseMass;
  input.velocity += input.constantAcceleration * timeScale;
  input.velocity *= input.damping;
  input.forceAccum = {};
  return input;
}

}  // namespace Physics