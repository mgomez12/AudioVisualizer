#include "SimulationApp.hpp"


#include "glm/gtx/string_cast.hpp"

#include "gloo/shaders/PhongShader.hpp"
#include "gloo/components/RenderingComponent.hpp"
#include "gloo/components/ShadingComponent.hpp"
#include "gloo/components/CameraComponent.hpp"
#include "gloo/components/LightComponent.hpp"
#include "gloo/components/MaterialComponent.hpp"
#include "gloo/MeshLoader.hpp"
#include "gloo/lights/PointLight.hpp"
#include "gloo/lights/DirectionalLight.hpp"
#include "gloo/lights/AmbientLight.hpp"
#include "gloo/cameras/ArcBallCameraNode.hpp"
#include "gloo/debug/AxisNode.hpp"

#include "VisNode.hpp"

#include "SimNodes.hpp"


namespace GLOO {
SimulationApp::SimulationApp(const std::string& app_name,
                             glm::ivec2 window_size,
                             float integration_step,
                             char* filename)
    : Application(app_name, window_size),
      integration_step_(integration_step),
      filename_(filename){
  // TODO: remove the following two lines and use integrator type and step to
  // create integrators; the lines below exist only to suppress compiler
  // warnings.

}

void SimulationApp::SetupScene() {
  SceneNode& root = scene_->GetRootNode();

  auto camera_node = make_unique<ArcBallCameraNode>(45.f, 0.75f, 10.0f);
  scene_->ActivateCamera(camera_node->GetComponentPtr<CameraComponent>());
  root.AddChild(std::move(camera_node));

  //root.AddChild(make_unique<AxisNode>('A'));

  auto ambient_light = std::make_shared<AmbientLight>();
  ambient_light->SetAmbientColor(glm::vec3(0.5f));
  root.CreateComponent<LightComponent>(ambient_light);

  auto point_light = std::make_shared<PointLight>();
  point_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  point_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  point_light->SetAttenuation(glm::vec3(1.0f, 0.09f, 0.032f));
  auto point_light_node = make_unique<SceneNode>();
  point_light_node->CreateComponent<LightComponent>(point_light);
  point_light_node->GetTransform().SetPosition(glm::vec3(0.0f, 5.0f, 10.f));

  auto directional_light = std::make_shared<DirectionalLight>();
  directional_light->SetDiffuseColor(glm::vec3(0.8f, 0.8f, 0.8f));
  directional_light->SetSpecularColor(glm::vec3(1.0f, 1.0f, 1.0f));
  directional_light->SetDirection(glm::vec3(0,-1.0f,-.2f));
  auto directional_light_node = make_unique<SceneNode>();
  directional_light_node->CreateComponent<LightComponent>(directional_light);

  auto vis_node = make_unique<VisNode>(filename_, integration_step_, 15, 5, 10, 1.8);
  root.AddChild(std::move(point_light_node));
  root.AddChild(std::move(vis_node));
  root.AddChild(std::move(directional_light_node));


}

}  // namespace GLOO
