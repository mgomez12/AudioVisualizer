#pragma once

#include "ShaderProgram.hpp"

namespace GLOO {
  // A simple shader for debug purposes.
  class ParticleShader : public ShaderProgram {
  public:
    ParticleShader(glm::vec3 color1, glm::vec3 color2, float size, std::vector<float>* lifespans);
    void SetTargetNode(const SceneNode& node,
      const glm::mat4& model_matrix) const override;
    void SetCamera(const CameraComponent& camera) const override;

  private:
    void AssociateVertexArray(VertexArray& vertex_array) const;
    glm::vec3 color1_;
    glm::vec3 color2_;
    std::vector<float>* lifespans_;
    float size_;
  };
}  // namespace GLOO
