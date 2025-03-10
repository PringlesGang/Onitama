#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../cli/game.h"
#include "../util/base64.h"
#include "../util/parse.h"
#include "../util/stateGraph.h"

namespace Experiments {
namespace StateGraph {

void Execute(StateGraphArgs args) {
  std::cout << std::format(
                   "Generating state graph for {}:\n",
                   Base64::Encode(args.StartingConfiguration->Serialize()))
            << *args.StartingConfiguration << std::endl;

  ::StateGraph::Graph graph =
      args.ImportPath ? ::StateGraph::Graph::Import(args.ImportPath.value())
                      : ::StateGraph::Graph();

  graph.Add(Game::Game(*args.StartingConfiguration));

  const std::shared_ptr<const ::StateGraph::Vertex> vertex =
      graph.Get(*args.StartingConfiguration)->lock();

  switch (vertex->Quality) {
    case WinState::Lost:
      std::cout << "Lost" << std::endl;
      break;
    case WinState::Unknown:
      std::cout << "Tie" << std::endl;
      break;
    case WinState::Won:
      std::cout << "Won" << std::endl;
      break;
  }

  if (args.ExportPath) graph.Export(args.ExportPath.value());
}

bool StateGraphArgs::Parse(std::istringstream& stream) {
  std::string argument;
  stream >> argument;
  Parse::ToLower(argument);

  if (argument.empty()) return true;

  if (argument == "--state" || argument == "-s") {
    const std::optional<Game::GameSerialization> serialization =
        Game::Game::ParseSerialization(stream);

    if (!serialization) return false;

    StartingConfiguration = std::make_shared<Game::Game>(
        std::move(Game::Game::FromSerialization(serialization.value())));

  } else if (argument == "--game" || argument == "-g") {
    Parse::GameConfiguration config;
    if (!config.Parse(stream) || !config.IsValid()) return false;
    StartingConfiguration =
        std::make_shared<Game::Game>(config.ToGame().value());

  } else if (argument == "--export" || argument == "-e") {
    const std::optional<std::filesystem::path> path = Parse::ParsePath(stream);

    if (!path) return false;

    ExportPath = path.value();

  } else if (argument == "--import" || argument == "-i") {
    const std::optional<std::filesystem::path> path = Parse::ParsePath(stream);

    if (!path) return false;

    ImportPath = path.value();

  } else {
    Parse::Unparse(stream, argument);
    return true;
  }

  return Parse(stream);
}

bool StateGraphArgs::IsValid() const {
  return StartingConfiguration != nullptr;
}

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  StateGraphArgs args;
  if (!args.Parse(command)) {
    std::cerr << "Failed to parse state graph experiment!" << std::endl;
    return std::nullopt;
  }

  if (!args.IsValid()) {
    std::cerr << "Invalid state graph arguments!" << std::endl;
    return std::nullopt;
  }

  return [args] { Execute(args); };
}

}  // namespace StateGraph
}  // namespace Experiments
