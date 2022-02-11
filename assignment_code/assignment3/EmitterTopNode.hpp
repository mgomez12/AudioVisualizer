#pragma once


#include "gloo/Application.hpp"

#include "Emitter.hpp"

namespace GLOO {
  class EmitterTopNode : public SceneNode {
  public:
    EmitterTopNode(double integration_step, std::vector<std::vector<float>>* pitches, int* curr_ix, std::vector<glm::vec4>* colors);
    void Update(double delta_time) override;
private:
  std::vector<std::vector<float>>* pitches_;
  int* curr_ix_;
  double integration_step_;
  bool prev_released_;
  std::vector<Emitter*> emitters_;
  std::vector<glm::vec4> colors_;
  int offset_;
  };

  
}