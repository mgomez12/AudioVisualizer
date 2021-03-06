#include <chrono>
#include <string>
#include <cstdio>
#include <stdexcept>
#include <math.h>
#include <complex>

#include "SimulationApp.hpp"
#include "IntegratorType.hpp"

using namespace GLOO;

int main(int argc, char** argv) {
  


  if (argc != 2) {
    printf("Usage invalid\n");
    return -1;
  }



  std::unique_ptr<SimulationApp> app = make_unique<SimulationApp>(
      "Audio Visualizer", glm::ivec2(1440, 900), .005, argv[1]);

  app->SetupScene();

  using Clock = std::chrono::high_resolution_clock;
  using TimePoint =
      std::chrono::time_point<Clock, std::chrono::duration<double>>;
  TimePoint last_tick_time = Clock::now();
  TimePoint start_tick_time = last_tick_time;
  while (!app->IsFinished()) {
    TimePoint current_tick_time = Clock::now();
    double delta_time = (current_tick_time - last_tick_time).count();
    double total_elapsed_time = (current_tick_time - start_tick_time).count();
    app->Tick(delta_time, total_elapsed_time);
    last_tick_time = current_tick_time;
  }
  return 0;
}
