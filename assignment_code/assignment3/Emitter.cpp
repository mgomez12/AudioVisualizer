#include "Emitter.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/ParticleShader.hpp"
#include "gloo/shaders/SimpleShader.hpp"


#define MAX_PER_EMITTER 120

namespace GLOO {
  Emitter::Emitter(double integration_step, std::vector<std::vector<float>>* pitches, double lifespan_seconds, int rate, glm::vec4 color)
    : integration_step_(integration_step),
      lifespan_seconds_(lifespan_seconds),
      rate_(rate),
      pitches_(pitches),
      color_(color)
      {


    auto particles = make_unique<VertexObject>();
    particles_ = particles.get();
    auto pos = make_unique<PositionArray>();
    lifespans_ = std::vector<float>();
    auto lifes = make_unique<LifespanArray>();
    

    system_ = make_unique<GravSystem>(6, 1);
    state_ = make_unique<ParticleState>();

    for (int i = 0; i < MAX_PER_EMITTER; i++) {
      state_->positions.push_back(glm::vec3(0, 0, 0));
      state_->velocities.push_back(glm::vec3(0, 0, 0));
      lifespans_.push_back(0);
      lifes->push_back(0);
      pos->push_back(glm::vec3(0, 0, 0));

    }

    time_since_last_ = 0;
    next_to_spawn_ = 1;
    particles->UpdatePositions(std::move(pos));
    particles->UpdateLifespans(std::move(lifes));
    CreateComponent<RenderingComponent>(std::move(particles));
    auto shader = make_unique<ParticleShader>(glm::vec3(color_.x, color_.y, color_.z), glm::vec3(1, 1, 0), 400, &lifespans_);
    CreateComponent<ShadingComponent>(std::move(shader));

  }

  void Emitter::Update(double delta_time) {
    for (int i = 0; i < (int)(delta_time / integration_step_); i++) {
      *state_ = integrator_->Integrate(*system_, *state_, time_, integration_step_);
      time_ += integration_step_;
    }

    for (int i = 0; i < lifespans_.size(); i++) {
      lifespans_[i] -= delta_time/lifespan_seconds_;
      if (lifespans_[i] < 0) {
        lifespans_[i] = 0;
      }
    }
    if (time_since_last_ > 1.f / (rate_*(val_)) && lifespans_[next_to_spawn_] == 0) {
      time_since_last_ = 0;
      lifespans_[next_to_spawn_] = 1;
      float x_var = (float)rand() / RAND_MAX * 1;
      float y_var = (float)rand() / RAND_MAX * 3;
      float z_var = (float)rand() / RAND_MAX * 3;
      state_->positions[next_to_spawn_] = glm::vec3(0, 0, 0);
      state_->velocities[next_to_spawn_] = val_* glm::vec3(x_var, 5 + y_var, 5 + z_var);
      next_to_spawn_ = (next_to_spawn_ < MAX_PER_EMITTER-1) ? next_to_spawn_ + 1 : 0;
    }
    auto lifes = make_unique<LifespanArray>();
    auto pos = make_unique<PositionArray>();
    for (int i = 0; i < lifespans_.size(); i++) {
      lifes->push_back(lifespans_[i]);
      pos->push_back(state_->positions[i]);
    }
    particles_->UpdateLifespans(std::move(lifes));
    particles_->UpdatePositions(std::move(pos));
    
    time_since_last_ += delta_time;
  }

  void Emitter::SetVal(float val) {
    val_ = val;
  }

}