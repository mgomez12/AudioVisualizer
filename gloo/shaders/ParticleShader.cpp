#include "ParticleShader.hpp"

#include <stdexcept>

#include <glm/gtc/quaternion.hpp>
#include <glm/matrix.hpp>

#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/SceneNode.hpp"

namespace GLOO {
  ParticleShader::ParticleShader(glm::vec3 color1, glm::vec3 color2, float size, std::vector<float>* lifespans)
    : ShaderProgram(std::unordered_map<GLenum, std::string>(
      { {GL_VERTEX_SHADER, "particle.vert"},
       {GL_FRAGMENT_SHADER, "particle.frag"} })),
    color1_(color1),
    color2_(color2),
    size_(size),
    lifespans_(lifespans){
    glEnable(GL_PROGRAM_POINT_SIZE);
  }

  void ParticleShader::AssociateVertexArray(VertexArray& vertex_array) const {
    if (!vertex_array.HasPositionBuffer()) {
      throw std::runtime_error("Simple shader requires vertex positions!");
    }
    vertex_array.LinkPositionBuffer(GetAttributeLocation("vertex_position"));
    vertex_array.LinkLifespanBuffer(GetAttributeLocation("vertex_lifespan"));
    vertex_array.SetDrawMode(DrawMode::Points);
  }

  void ParticleShader::SetTargetNode(const SceneNode& node,
    const glm::mat4& model_matrix) const {
    glEnable(GL_BLEND);
    glDepthMask(GL_FALSE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    // Associate the right VAO before rendering.
    AssociateVertexArray(node.GetComponentPtr<RenderingComponent>()
      ->GetVertexObjectPtr()
      ->GetVertexArray());

    // Set transform.
    SetUniform("model_matrix", model_matrix);
    SetUniform("size", size_); 
    SetUniform("color1", color1_);
    SetUniform("color2", color2_);
  }

  void ParticleShader::SetCamera(const CameraComponent& camera) const {
    SetUniform("view_matrix", camera.GetViewMatrix());
    SetUniform("projection_matrix", camera.GetProjectionMatrix());
  }

}  // namespace GLOO
