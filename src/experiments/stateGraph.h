#pragma once

#include <filesystem>

#include "../cli/command.h"
#include "../game/game.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view Command =
    "stategraph --state game_serialization (--export filepath) (--import "
    "filepath) (--intermediate path seconds-interval)\n"

    "stategraph --game (--duplicate-cards) (--cards set_aside r1 r2 b1 b2) "
    "(--size width height) (--export nodes-path edges-path) "
    "(--import nodes-path edges-path) (--strategy strategy) "
    "(--intermediate path seconds-interval)\n"

    "stategraph --load intermediate_path (--export nodes-path edges-path) "
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
  RetrogradeAnalysis,
};

struct StateGraphArgs {
  bool Parse(std::istringstream& stream);
  bool IsValid() const;

  static std::optional<StateGraphType> ParseStateGraphType(
      std::istringstream& stream);

  std::shared_ptr<Game::Game> StartingConfiguration = nullptr;
  std::optional<std::filesystem::path> LoadPath = std::nullopt;

  std::optional<std::pair<std::filesystem::path, std::filesystem::path>>
      ExportPaths = std::nullopt;
  std::optional<std::pair<std::filesystem::path, std::filesystem::path>>
      ImportPaths = std::nullopt;
  std::optional<std::filesystem::path> IntermediatePath = std::nullopt;

  std::optional<std::filesystem::path> ImagesPath = std::nullopt;

  size_t SaveTimeInterval = 0;

  StateGraphType Type = StateGraphType::Component;
};

void Execute(StateGraphArgs args);
void ExecuteLoad(StateGraphArgs args);

std::optional<Cli::Thunk> Parse(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments
