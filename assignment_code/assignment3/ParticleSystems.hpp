#pragma once
#include "ParticleSystemBase.hpp"

namespace GLOO {
  class PolySystem :
    public ParticleSystemBase
  {
  public:

    PolySystem(float k, float drag);
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override;


    float k_;
    float drag_;
    float default_dist = glm::length(glm::vec3(0, 1, (1.f + sqrt(5)) / 2));
  };

  class GravSystem :
    public ParticleSystemBase
  {
  public:

    GravSystem(float g, float drag);
    ParticleState ComputeTimeDerivative(const ParticleState& state, float time) const override;


    float g_;
    float drag_;
  };

}