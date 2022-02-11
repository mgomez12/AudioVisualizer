#include "SimNodes.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/shaders/PhongShader.hpp"
#include "gloo/InputManager.hpp"
#include "gloo/shaders/SimpleShader.hpp"

namespace GLOO {
  PolyNode::PolyNode(double integration_step, double k, double drag, float attack, float transient_target, std::vector<std::vector<float>>* bands, int* curr_ix, std::vector<glm::vec4>* colors_v)
    : integration_step_(integration_step),
      transient_target_(transient_target),
      bands_(bands),
      curr_ix_(curr_ix),
      colors_(*colors_v),
      attack_(attack) {

    auto phong_shader = std::make_shared<PhongShader>();
    std::unique_ptr<PositionArray> iso_positions = make_unique<PositionArray>(points_);
    std::unique_ptr<IndexArray> iso_indices = make_unique<IndexArray>(indices_);
    std::unique_ptr<NormalArray> iso_normals = make_unique<NormalArray>(points_.size());
    std::unique_ptr<ColorArray> colors = make_unique<ColorArray>(colors_);
    for (int i = 0; i < iso_indices->size(); i += 3) {
      int ix_0 = (*iso_indices)[i];
      int ix_1 = (*iso_indices)[i + 1];
      int ix_2 = (*iso_indices)[i + 2];
      glm::vec3 norm = glm::cross(((*iso_positions)[ix_0] - (*iso_positions)[ix_1]), ((*iso_positions)[ix_2] - (*iso_positions)[ix_1]));
      (*iso_normals)[ix_0] += norm / 5.f;
      (*iso_normals)[ix_1] += norm / 5.f;
      (*iso_normals)[ix_2] += norm / 5.f;
    }

    for (auto color : *colors) {
      std::cout << color.x << "," << color.y << ", " << color.z << std::endl;
    }

    system_ = make_unique<PolySystem>(k, drag);
    state_ = make_unique<ParticleState>();
    for (int i = 0; i < points_.size(); i++) {
      hit_target_.push_back(true);
      target_.push_back(0);
      state_->positions.push_back(points_[i]);
      state_->velocities.push_back(glm::vec3(0, 0, 0));
      transient_targets_.push_back(transient_target);
    }

    auto poly_node = make_unique<SceneNode>();
    auto poly_obj = std::make_shared<VertexObject>();
    auto shader = std::make_shared<PhongShader>();
    poly_obj->UpdateIndices(std::move(iso_indices));
    poly_obj->UpdatePositions(std::move(iso_positions));
    poly_obj->UpdateNormals(std::move(iso_normals));
    poly_obj->UpdateColors(std::move(colors));
    poly_node->CreateComponent<RenderingComponent>(poly_obj);
    poly_node->CreateComponent<ShadingComponent>(shader);
    poly_node_ = poly_node.get();
    poly_obj_ = poly_obj.get();
    AddChild(std::move(poly_node));
    poly_node_->GetTransform().SetScale(glm::vec3(.7f, .7f, .7f));

    rotations_ = true;
    diff_ = glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1);
  }

  void PolyNode::Update(double delta_time) {
      //std::cout << "updating, pos is " << state_->positions[0].x << ", " << state_->positions[0].y << ", " << state_->positions[0].z << std::endl;
   
    std::vector<float> scaled_bands;
    std::vector<float> band = bands_->at(*curr_ix_);
    float cutoff = .4;
    float division = .2;
    for (int i = 0; i < points_.size(); i++) {
      float scaled = band[i] / 200;
      scaled_bands.push_back(1 + (scaled < cutoff ? scaled * (division/cutoff) : division + (scaled-cutoff)));
    }
    Trigger(scaled_bands);
    if (InputManager::GetInstance().IsKeyPressed('S')) {
      if (prev_released_) {
        rotations_ = !rotations_;
      }
      prev_released_ = false;
    }
    else if (InputManager::GetInstance().IsKeyPressed('A')) {
      if (prev_released_) {
        poly_node_->SetActive(!poly_node_->IsActive());
      }
      prev_released_ = false;
    }
    else prev_released_ = true;


    for (int i = 0; i < (int)(delta_time / integration_step_); i++) {
      *state_ = integrator_->Integrate(*system_, *state_, time_, integration_step_);
      time_ += integration_step_;
    }
    auto positions_new = make_unique<PositionArray>();
    for (int i = 0; i < state_->positions.size(); i++) {
      if (!hit_target_[i] && glm::length(state_->positions[i]) < glm::length(target_[i] * points_[i])) {
        state_->velocities[i] = attack_ * points_[i];
      }
      else if (!hit_target_[i]) {
        state_->velocities[i] = glm::vec3(0, 0, 0);
        state_->positions[i] = target_[i] * points_[i];
        hit_target_[i] = true;
        target_[i] = 0;
      }
      positions_new->push_back(state_->positions[i]);
    }
    poly_obj_->UpdatePositions(std::move(positions_new));
    if (rotations_) {
      a += a_d;
      b += b_d;
      c += c_d;
      diff_ = glm::mat4(cos(a) * cos(b), sin(a) * cos(b), -sin(b), 0, cos(a) * sin(b) * sin(c) - sin(a) * cos(c), sin(a) * sin(b) * sin(c) + cos(a) * cos(c), cos(b) * sin(c), 0, cos(a) * sin(b) * cos(c) + sin(a) * sin(c), sin(a) * sin(b) * cos(c) - cos(a) * sin(c), cos(b) * cos(c), 0, 0, 0, 0, 1);
      GetTransform().SetMatrix4x4(diff_);
    }
  }

  void PolyNode::Trigger(std::vector<float> targets) {
    for (int i = 0; i < points_.size(); i++) {
      if ((!hit_target_[i] && target_[i] > targets[i]) || glm::length(state_->positions[i]) > glm::length(targets[i] * points_[i])) continue;
      state_->velocities[i] = attack_ * points_[i];
      hit_target_[i] = false;
      target_[i] = targets[i];
    }
  }

  void PolyNode::TriggerTransients() {
    Trigger(transient_targets_);
    a_d = (rand() / RAND_MAX + 1) * glm::pi<float>() / 1000;
    b_d = (rand() / RAND_MAX + 1) * glm::pi<float>() / 1000;
    c_d = (rand() / RAND_MAX + 1) * glm::pi<float>() / 1000;
    
    
  }



 
}