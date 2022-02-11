#ifndef SIMULATION_APP_H_
#define SIMULATION_APP_H_

#include "gloo/Application.hpp"

#include "IntegratorType.hpp"

namespace GLOO {
class SimulationApp : public Application {
 public:
  SimulationApp(const std::string& app_name,
                glm::ivec2 window_size,
                float integration_step,
                char* filename);
  void SetupScene() override;

 private:
  float integration_step_;
  char* filename_;

  std::unique_ptr<PositionArray> iso_positions_;
  std::unique_ptr<IndexArray> iso_indices_;
  std::unique_ptr<NormalArray> iso_normals_;
};
}  // namespace GLOO

#endif
