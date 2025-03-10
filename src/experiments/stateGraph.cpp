#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../cli/game.h"
#include "../util/base64.h"
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

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  StateGraphArgs args;

  // Parse
  while (true) {
    std::string argument;
    command >> argument;
    Cli::Command::ToLower(argument);

    if (argument.empty()) break;

    if (argument == "--state" || argument == "-s") {
      const std::optional<Game::GameSerialization> serialization =
          ParseSerialization(command);

      if (!serialization) return std::nullopt;

      args.StartingConfiguration = std::make_shared<Game::Game>(
          std::move(Game::Game::FromSerialization(serialization.value())));

    } else if (argument == "--game" || argument == "-g") {
      if (!ParseGame(command, args)) return std::nullopt;

    } else if (argument == "--export" || argument == "-e") {
      const std::optional<std::filesystem::path> path = ParsePath(command);

      if (!path) return std::nullopt;

      args.ExportPath = path.value();

    } else if (argument == "--import" || argument == "-i") {
      const std::optional<std::filesystem::path> path = ParsePath(command);

      if (!path) return std::nullopt;

      args.ImportPath = path.value();

    } else {
      std::cout << std::format("Unknown argument \"{}\"!", argument)
                << std::endl;
      return std::nullopt;
    }
  }

  // Validate
  if (args.StartingConfiguration == nullptr) {
    std::cout << "No starting state provided!" << std::endl;
    return std::nullopt;
  }

  return [args] { Execute(args); };
}

bool ParseGame(std::istringstream& command, StateGraphArgs& args) {
  Cli::GameArgs gameArgs;

  std::string argument;
  command >> argument;
  Cli::Command::ToLower(argument);

  while (!argument.empty()) {
    if (argument == "--duplicate-cards" || argument == "-d") {
      gameArgs.RepeatCards = true;

    } else if (argument == "--cards" || argument == "-c") {
      if (!Cli::GameCommand::ParseCards(command, gameArgs)) return false;

    } else if (argument == "--size" || argument == "-s") {
      if (!Cli::GameCommand::ParseDimensions(command, gameArgs)) return false;

    } else {
      Cli::Command::Unparse(command, argument);
      break;
    }

    argument.clear();
    command >> argument;
  }

  args.StartingConfiguration = std::make_shared<Game::Game>(gameArgs.ToGame());
  return true;
}

std::optional<Game::GameSerialization> ParseSerialization(
    std::istringstream& command) {
  std::string serializationString;
  command >> serializationString;

  if (serializationString.empty()) {
    std::cout << "No game serialization provided!" << std::endl;
    return std::nullopt;
  }

  Game::GameSerialization serialization;
  try {
    return Base64::Decode<Game::GAME_SERIALIZATION_SIZE>(
        std::move(serializationString));
  } catch (std::runtime_error err) {
    std::cout << std::format("Failed to parse base64 string \"{}\"",
                             serializationString)
              << std::endl;
    return std::nullopt;
  }
}

std::optional<std::filesystem::path> ParsePath(std::istringstream& command) {
  std::string path;
  command >> path;

  if (path.empty()) {
    std::cout << "No filepath provided!" << std::endl;
    return std::nullopt;
  }

  try {
    return std::filesystem::path(path);
  } catch (std::exception e) {
    std::cout << "Failed to construct filepath: " << e.what() << std::endl;
    return std::nullopt;
  }
}

}  // namespace StateGraph
}  // namespace Experiments
