#pragma once

#include "gloo/Application.hpp"
#include "ParticleState.hpp"

#include "Integrators.hpp"
#include "ParticleSystems.hpp"

namespace GLOO {
  class PolyNode : public SceneNode {
  public:
    PolyNode(double integration_step, double k, double drag, float attack, float transient_target, std::vector<std::vector<float>>* bands, int* curr_ix, std::vector<glm::vec4>* colors);

  void Update(double delta_time) override;
  void Trigger(std::vector<float> targets);
  void TriggerTransients();

  private:
    float gr = (1.f + sqrt(5)) / 2;
    std::unique_ptr<PolySystem> system_;
    std::unique_ptr<ParticleState> state_;
    std::unique_ptr<IntegratorBase<PolySystem, ParticleState>> integrator_ = make_unique<RK4Integrator<PolySystem, ParticleState>>();
    double time_;
    double integration_step_;
    SceneNode* poly_node_;
    VertexObject* poly_obj_;
    bool prev_released_;
    float attack_;
    int* curr_ix_;
    bool rotations_;
    std::vector<float> transient_targets_;
    std::vector<std::vector<float>>* bands_;
    std::vector<bool> hit_target_;
    float transient_target_;
    std::vector<float> target_;
    glm::mat4 diff_;
    float a = 0;
    float b = 0;
    float c = 0;
    float a_d = 0;
    float b_d = 0;
    float c_d = 0;
    std::vector<glm::vec4> colors_;
    std::vector<glm::vec3> points_ = { glm::vec3(0, 1, gr),
                                   glm::vec3(-gr, 0,1),
                                   glm::vec3(0,-1, gr),
                                   glm::vec3(gr, 0, 1),
                                   glm::vec3(1, gr, 0),
                                   glm::vec3(1,-gr, 0),
                                   glm::vec3(-1, gr, 0),
                                   glm::vec3(-1, -gr, 0),
                                   
                                   glm::vec3(0,-1,-gr),
                                  glm::vec3(0, 1,-gr),
                                   glm::vec3(gr, 0,-1),
                                   glm::vec3(-gr, 0,-1)
    };

    std::vector<unsigned int> indices_ = { 0,2,1,
                                 0,4,3,
                                 0,6,4,
                                 0,3,2,
                                 0,1,6,
                                 9,8,10,
                                 9,11,8,
                                 9,4,6,
                                 9,10,4,
                                 9,6,11,
                                 2,3,5,
                                 2,5,7,
                                 2,7,1,
                                 8,7,5,
                                 8,5,10,
                                 8,11,7,
                                 4,10,3,
                                 5,3,10,
                                 6,1,11,
                                 7,11,1
    };

  };

}