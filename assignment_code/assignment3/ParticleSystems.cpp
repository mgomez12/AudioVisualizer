#include "ParticleSystems.hpp"
#include <iostream>

#define RANDOM_VAL (((float)rand()/RAND_MAX) -.5f)

namespace GLOO {
  PolySystem::PolySystem(float k, float drag): k_(k), drag_(drag) {};

  ParticleState PolySystem::ComputeTimeDerivative(const ParticleState& state, float time) const  {
    auto new_state = ParticleState();
    for (int i = 0; i < state.positions.size(); i++) {
      new_state.positions.push_back(state.velocities[i]);
      new_state.velocities.push_back(glm::vec3(0, 0, 0));
      new_state.velocities[i] += -k_ * (glm::length(state.positions[i]) - default_dist) * glm::normalize(state.positions[i]);
      new_state.velocities[i] += -drag_ * state.velocities[i];
    }
    return new_state;
  };

  GravSystem::GravSystem(float g, float drag) : g_(g), drag_(drag) {};

  ParticleState GravSystem::ComputeTimeDerivative(const ParticleState& state, float time) const {
    auto new_state = ParticleState();
    for (int i = 0; i < state.positions.size(); i++) {
      new_state.positions.push_back(state.velocities[i]);
      new_state.velocities.push_back(glm::vec3(0, 0, 0));
      new_state.velocities[i] += glm::vec3(0,-g_,0) - drag_ * state.velocities[i];
    }
    return new_state;
  };

}