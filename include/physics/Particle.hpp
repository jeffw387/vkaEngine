#pragma once
#include <glm/glm.hpp>
#include "Clock.hpp"
#include <chrono>

namespace Physics {
using namespace std::chrono_literals;
class Particle {
public:
  glm::vec3 position;
  glm::vec3 velocity;
  glm::vec3 acceleration;
  float damping;

protected:
  float inverseMass;
};

inline Particle integrate(Particle input, glm::vec3 newForces, float deltaTimeSeconds) {
  input.position += input.velocity * deltaTimeSeconds;
  input.acceleration += newForces * deltaTimeSeconds;
  input.velocity += input.acceleration * deltaTimeSeconds;
  
}

inline Particle updateVelocity(Particle input, float deltaTimeSeconds) {
  input.velocity = input.velocity * input.damping + input.acceleration * deltaTimeSeconds;
}
}  // namespace Physics