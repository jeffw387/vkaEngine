#include <algorithm>
#include "physics/Particle.hpp"

namespace Physics {
void Particle::setMass(float mass) { inverseMass = 1.f / mass; }

void ParticleForceRegistry::add(
    Particle* particle,
    ParticleForceGenerator* generator) {
  registry.emplace_back(ParticleForceRegistration{particle, generator});
}

void ParticleForceRegistry::remove(
    Particle* particle,
    ParticleForceGenerator* generator) {
  std::remove_if(
      registry.begin(), registry.end(), [&](const auto& registration) {
        return registration.particle == particle &&
               registration.generator == generator;
      });
}

void ParticleForceRegistry::clear() { registry.clear(); }

void ParticleForceRegistry::updateForces(float duration) {
  for (auto& [particle, generator] : registry) {
    generator->updateForce(particle, duration);
  }
}
}  // namespace Physics