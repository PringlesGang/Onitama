#pragma once

#include "../util/stopwatch.h"
#include "stateGraph.h"

namespace StateGraph {
namespace SaveSystem {

using namespace std::chrono_literals;

struct SaveParameters {
  std::filesystem::path SavePath;
  std::chrono::duration<size_t> SaveInterval = 0s;
  Stopwatch<> SaveTimer = Stopwatch(false);

  Stopwatch<> RuntimeTimer = Stopwatch(false);

  static std::optional<SaveParameters> Parse(std::istringstream& stream);

  void Update();

  void StartTimers();
  void UnpauseTimers();
  void PauseTimers();

  bool ShouldSave();
  void Save(Graph& graph);
};

}  // namespace SaveSystem
}  // namespace StateGraph
