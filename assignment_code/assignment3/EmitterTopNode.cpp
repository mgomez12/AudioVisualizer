#include "EmitterTopNode.hpp"
#include "Emitter.hpp"
#include "gloo/InputManager.hpp"

namespace GLOO {
  EmitterTopNode::EmitterTopNode(double integration_step, std::vector<std::vector<float>>* pitches, int* curr_ix, std::vector<glm::vec4>* colors)
  : curr_ix_(curr_ix),
    pitches_(pitches),
    colors_(*colors),
    integration_step_(integration_step){

    float pi = glm::pi<float>();
    for (int i = 0; i < 12; i++) {
      auto emitter = make_unique<Emitter>(integration_step, pitches_, 2, 20, colors_[i]);
      emitters_.push_back(emitter.get());
      AddChild(std::move(emitter));
      float angle = -((i-5)*pi / 11 - pi/22);
      emitters_[i]->SetActive(false);
      emitters_[i]->GetTransform().SetPosition(4.f *glm::vec3(cos(angle+pi/2), -.5, -sin(angle+pi/2)) - glm::vec3(0,0,1));
      emitters_[i]->GetTransform().SetRotation(glm::quat(cos(angle / 2), 0, sin(angle / 2), 0));
    }
    offset_ = 0;

  }

  void EmitterTopNode::Update(double delta_time) {
    int max_ix = -1;
    float m = 0;
    for (int i = 0; i < 12; i++) {
      float a = (pitches_->at(*curr_ix_))[i + offset_];
      if (a > m) {
        max_ix = i;
        m = a;
      }
      emitters_[i]->SetVal(((a-2) < 0 ? 0 : a-2)/5);
    }
    if (m > 4) emitters_[max_ix]->SetVal(3);
    if (InputManager::GetInstance().IsKeyPressed('A')) {
      if (prev_released_) {
        for (int i = 0; i < 12; i++) emitters_[i]->SetActive(!emitters_[i]->IsActive());
      }
      prev_released_ = false;
    }
    else if (InputManager::GetInstance().IsKeyPressed('Q')) {
      if (prev_released_) {
        offset_ = 0;
      }
      prev_released_ = false;
    }
    else if (InputManager::GetInstance().IsKeyPressed('W')) {
      if (prev_released_) {
        offset_ = 12;
      }
      prev_released_ = false;
    }
    else if (InputManager::GetInstance().IsKeyPressed('E')) {
      if (prev_released_) {
        offset_ = 24;
      }
      prev_released_ = false;
    }
    else prev_released_ = true;


  }
}

