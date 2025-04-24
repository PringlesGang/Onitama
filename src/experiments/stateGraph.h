#pragma once

#include <filesystem>

#include "../cli/command.h"
#include "../game/game.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view Command =
    "stategraph state game_serialization (--export filepath) "
    "(--import filepath) (--intermediate path seconds-interval)\n"

    "stategraph game (--duplicate-cards) (--cards set_aside r1 r2 b1 b2) "
    "(--size width height) (--export nodes-path edges-path) "
    "(--import nodes-path edges-path) (--strategy strategy) "
    "(--intermediate path seconds-interval)\n"

    "stategraph load intermediate_path (--export nodes-path edges-path) "
    "(--strategy strategy) (--intermediate path seconds-interval)";

constexpr inline std::string_view Description =
    "Depending on the arguments, "
    "will construct the state graph for either:\n"
    "- the provided game state;\n"
    "- the starting configuration with the supplied arguments;\n"
    "- the loaded intermediate state graph.\n\n"

    "Will export the graph to the provided filepath.\n"
    "Will import the graph from the provided filepath.\n"
    "Will repeatedly save intermediate results to the provided filepath after "
    "the supplied duration.\n\n"

    "--strategy will construct the state graph through the given strategy.";

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

  bool IsValid() const override {
    return StartingConfiguration != nullptr || LoadPath.has_value();
  }

  bool Parse(std::istringstream& stream);
  void Execute() const override;

  size_t SaveTimeInterval = 0;
  std::optional<std::filesystem::path> IntermediatePath = std::nullopt;
  std::optional<std::filesystem::path> LoadPath = std::nullopt;
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

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments
