#pragma once

#include <filesystem>

#include "../../cli/command.h"
#include "../../game/game.h"
#include "../../stateGraph/strategies.h"

namespace Experiments {
namespace StateGraph {

using namespace ::StateGraph::SaveSystem;

enum class StateGraphType {
  Component,
  ForwardRetrogradeAnalysis,
  DispersedFrontier,
};

struct StateGraphArgs {
 public:
  virtual StateGraphType GetType() const = 0;

  virtual bool IsValid() const { return StartingConfiguration != nullptr; }

  static std::optional<std::shared_ptr<StateGraphArgs>> Parse(
      std::istringstream& stream);
  static std::optional<StateGraphType> ParseStateGraphType(
      std::istringstream& stream);

  virtual void Execute() const = 0;

  std::shared_ptr<Game::Game> StartingConfiguration = nullptr;

  std::optional<std::pair<std::filesystem::path, std::filesystem::path>>
      ExportPaths = std::nullopt;
  std::optional<std::pair<std::filesystem::path, std::filesystem::path>>
      ImportPaths = std::nullopt;

  std::optional<std::filesystem::path> ImagesPath = std::nullopt;

  std::optional<SaveParameters> IntermediateParameters = std::nullopt;

 protected:
  bool ParseCommonArgs(std::istringstream& stream);
};

struct ComponentArgs : public StateGraphArgs {
  StateGraphType GetType() const override { return StateGraphType::Component; }

  bool Parse(std::istringstream& stream) { return true; }
  void Execute() const override;
};

struct ForwardRetrogradeAnalysisArgs : public StateGraphArgs {
  StateGraphType GetType() const override {
    return StateGraphType::ForwardRetrogradeAnalysis;
  }

  bool Parse(std::istringstream& stream) { return true; };
  void Execute() const override;
};

struct DispersedFrontierArgs : public StateGraphArgs {
  StateGraphType GetType() const override {
    return StateGraphType::DispersedFrontier;
  }

  bool IsValid() const override {
    return StateGraphArgs::IsValid() && Depth > 0 && MaxThreadCount > 0;
  }

  bool Parse(std::istringstream& stream);
  void Execute() const override;

  size_t Depth = 0;
  size_t MaxThreadCount = 0;
};

}  // namespace StateGraph
}  // namespace Experiments
