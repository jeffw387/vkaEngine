#include <algorithm>
#include <cmath>
#include "physics/Particle.hpp"

namespace Physics {
void Particle::setMass(float mass) { inverseMass = 1.f / mass; }

bool Particle::hasInfiniteMass() const { return inverseMass == 0; }

void ParticleGravityGenerator::updateForce(Particle* particle, float duration) {
  if (particle->hasInfiniteMass()) {
    return;
  }
  particle->forceAccum += gravity * particle->inverseMass;
}

void ParticleDragGenerator::updateForce(Particle* particle, float duration) {
  glm::vec3 force = particle->velocity;
  float dragCoefficient = force.length();
  dragCoefficient =
      (k1 * dragCoefficient) + (k2 * dragCoefficient * dragCoefficient);
  force = glm::normalize(force);
  force *= -dragCoefficient;
  particle->forceAccum += force;
}

void ParticleSpring::updateForce(Particle* particle, float duration) {
  glm::vec3 force = particle->velocity;
  force -= other->position;
  float magnitude = force.length();
  magnitude = std::abs(magnitude - restLength);
  magnitude *= springConstant;
  force = glm::normalize(force);
  force *= -magnitude;
  particle->forceAccum += force;
}

void ParticleForceRegistry::add(
    Particle* particle,
    ParticleForceGenerator* generator) {
  registry.emplace_back(ParticleForceRegistration{particle, generator});
}

void ParticleForceRegistry::remove(
    Particle* particle,
    ParticleForceGenerator* generator) {
  registry.erase(std::remove_if(
      registry.begin(), registry.end(), [&](const auto& registration) {
        return registration.particle == particle &&
               registration.generator == generator;
      }));
}

void ParticleForceRegistry::clear() { registry.clear(); }

void ParticleForceRegistry::updateForces(float duration) {
  for (auto& [particle, generator] : registry) {
    generator->updateForce(particle, duration);
  }
}
}  // namespace Physics