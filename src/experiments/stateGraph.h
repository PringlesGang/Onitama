#pragma once

#include <filesystem>

#include "../cli/command.h"
#include "../game/game.h"

namespace Experiments {
namespace StateGraph {

constexpr inline std::string_view Name = "stategraph";

constexpr inline std::string_view Command =
    "stategraph --state game_serialization (--export filepath) (--import "
    "filepath)\n"

    "stategraph --game (--duplicate-cards) (--cards set_aside r1 r2 b1 b2) "
    "(--size width height) (--export filepath) (--import filepath)";

constexpr inline std::string_view Description =
    "Depending on the arguments, "
    "will construct the winning state graph for either:\n"
    "- the provided game state;\n"
    "- or the starting configuration with the supplied arguments.\n\n"

    "Will export the graph to the provided filepath.\n"
    "Will import the graph from the provided filepath.";

struct StateGraphArgs {
  std::shared_ptr<Game::Game> StartingConfiguration = nullptr;

  std::optional<std::filesystem::path> ExportPath = std::nullopt;
  std::optional<std::filesystem::path> ImportPath = std::nullopt;
};

void Execute(StateGraphArgs args);

std::optional<Cli::Thunk> Parse(std::istringstream& command);

bool ParseGame(std::istringstream& command, StateGraphArgs& args);
std::optional<Game::GameSerialization> ParseSerialization(
    std::istringstream& command);

std::optional<std::filesystem::path> ParsePath(std::istringstream& command);

}  // namespace StateGraph
}  // namespace Experiments
