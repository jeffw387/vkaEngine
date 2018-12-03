#include "Particle.hpp"

namespace Physics {
void Particle::setMass(float mass) {
  inverseMass = 1.f / mass;
}
}