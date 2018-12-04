#include <algorithm>
#include <cmath>
#include "physics/Particle.hpp"

namespace Physics {
void Particle::setMass(float mass) { inverseMass = 1.f / mass; }

bool Particle::hasInfiniteMass() const { return inverseMass == 0; }

void Particle::integrate(float timeScale) {
  position += velocity * timeScale;
  constantAcceleration += forceAccum * inverseMass;
  velocity += constantAcceleration * timeScale;
  velocity *= damping;
  forceAccum = {};
}

void ParticleContact::resolve(float duration) {
  float totalInverseMass = getTotalInverseMass();
  if (totalInverseMass > 0) {
    resolveVelocity(duration, totalInverseMass);
    if (penetration > 0) {
      resolveInterpenetration(duration, totalInverseMass);
    }
  }
}

float ParticleContact::calculateSeparatingVelocity() const {
  glm::vec3 relativeVelocity = particle[0]->velocity;
  if (particle[1]) {
    relativeVelocity -= particle[1]->velocity;
  }
  return glm::dot(relativeVelocity, contactNormal);
}

float ParticleContact::getTotalInverseMass() const {
  float totalInverseMass = particle[0]->inverseMass;
  if (particle[1]) {
    totalInverseMass += particle[1]->inverseMass;
  }
  return totalInverseMass;
}

void ParticleContact::resolveVelocity(float duration, float totalInverseMass) {
  float separatingVelocity = calculateSeparatingVelocity();
  if (separatingVelocity > 0) {
    return;
  }

  float newSeparatingVelocity = -separatingVelocity * restitution;
  float deltaVelocity = newSeparatingVelocity - separatingVelocity;

  float impulse = deltaVelocity / totalInverseMass;
  glm::vec3 impulsePerInverseMass = contactNormal * impulse;

  particle[0]->velocity += impulsePerInverseMass * particle[0]->inverseMass;

  if (particle[1]) {
    particle[1]->velocity += impulsePerInverseMass * particle[1]->inverseMass;
  }
}

void ParticleContact::resolveInterpenetration(
    float duration,
    float totalInverseMass) {
  glm::vec3 movePerInverseMass =
      contactNormal * (-penetration / totalInverseMass);

  particle[0]->position += movePerInverseMass * particle[0]->inverseMass;
  if (particle[1]) {
    particle[1]->position += movePerInverseMass * particle[1]->inverseMass;
  }
}

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

void ParticleAnchoredSpring::updateForce(Particle* particle, float duration) {
  glm::vec3 force = particle->velocity;
  force -= anchor;

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