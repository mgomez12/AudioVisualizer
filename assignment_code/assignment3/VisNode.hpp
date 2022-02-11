#pragma once
#include <chrono>
#include "gloo/Application.hpp"
#include "SimNodes.hpp"
#include "EmitterTopNode.hpp"

namespace GLOO {
  using Clock = std::chrono::high_resolution_clock;
  using TimePoint =
    std::chrono::time_point<Clock, std::chrono::duration<double>>;
  class VisNode : public SceneNode {
  public:
    VisNode(char* filename, double integration_step, double k, double drag, float attack, float transient_target);

    void Update(double delta_time) override;

  private:
    void SetupAudio(std::string filename);
    PolyNode* poly_node_;
    EmitterTopNode* emitter_node_;
    TimePoint begin_;
    std::vector<int> peaks_;
    std::vector<std::vector<float>> bands_;
    std::vector<std::vector<float>> pitches_;
    int curr_ix_;
    int next_peak_;
    int sample_rate_;

    std::vector<glm::vec4> colors_ = { glm::vec4(1,0,0,1),
                                        glm::vec4(1,.18,0,1),
                                        glm::vec4(1,.42,0,1),
                                        glm::vec4(1,.67, 0,1),
                                        glm::vec4(1,.85,0,1),
                                        glm::vec4(1,1,0,1),
                                        glm::vec4(.5, 1, 0,1),
                                        glm::vec4(0,1,0,1),
                                        glm::vec4(0,1,1,1),
                                        glm::vec4(0,0,1,1),
                                        glm::vec4(.5, 0, 1,1),
                                        glm::vec4(1,1,1,1)

    };
  };
}