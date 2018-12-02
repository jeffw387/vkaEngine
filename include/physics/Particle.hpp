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

inline Particle updatePosition(Particle input, vka::Clock::duration deltaTime) {
  input.position += input.velocity *
}
}