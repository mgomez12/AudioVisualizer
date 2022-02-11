#pragma once


#include "gloo/Application.hpp"
#include "ParticleState.hpp"

#include "Integrators.hpp"
#include "ParticleSystems.hpp"


namespace GLOO {
  class Emitter : public SceneNode {
  public:
    Emitter(double integration_step, std::vector<std::vector<float>>* pitches, double lifespan_seconds, int rate, glm::vec4 color);
    void Update(double delta_time) override;
    void SetVal(float val);

  private:
    double integration_step_;
    VertexObject* particles_;
    std::vector<glm::vec3> positions_;
    std::vector<float> lifespans_;
    int next_to_spawn_;
    float lifespan_seconds_;
    float val_;
    int rate_;
    std::unique_ptr<GravSystem> system_;
    std::unique_ptr<ParticleState> state_;
    double time_since_last_;
    std::unique_ptr<IntegratorBase<GravSystem, ParticleState>> integrator_ = make_unique<RK4Integrator<GravSystem, ParticleState>>();
    double time_;
    std::vector<std::vector<float>>* pitches_;
    glm::vec4 color_;
  };
}