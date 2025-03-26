#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../cli/game.h"
#include "../stateGraph/stateGraph.h"
#include "../util/base64.h"
#include "../util/parse.h"

namespace Experiments {
namespace StateGraph {

void Execute(StateGraphArgs args) {
  std::cout << "Generating state graph for:\n"
            << *args.StartingConfiguration << std::endl;

  ::StateGraph::Graph graph =
      args.ImportPaths ? ::StateGraph::Graph::Import(args.ImportPaths->first,
                                                     args.ImportPaths->second)
                       : ::StateGraph::Graph();

  std::function<void(Game::Game&&)> analyse;
  switch (args.Type) {
    case StateGraphType::Component:
      analyse = [&graph](Game::Game&& game) {
        graph.ExploreComponent(std::move(game));
      };
      break;

    case StateGraphType::ForwardRetrogradeAnalysis:
      analyse = [&graph](Game::Game&& game) {
        graph.ForwardRetrogradeAnalysis(std::move(game));
      };
      break;

    case StateGraphType::RetrogradeAnalysis:
      analyse = [&graph](Game::Game&& game) {
        graph.RetrogradeAnalysis(std::move(game));
      };
      break;

    default:
      std::cerr << std::format("Unknown state graph type \"{}\"",
                               (size_t)args.Type)
                << std::endl;
      return;
  }

  switch (args.Type) {
    default:
    case StateGraphType::Component:
      analyse(Game::Game(*args.StartingConfiguration));
      break;

    case StateGraphType::ForwardRetrogradeAnalysis:
    case StateGraphType::RetrogradeAnalysis: {
      analyse(Game::Game(*args.StartingConfiguration));

      const std::shared_ptr<const ::StateGraph::Vertex> vertex =
          graph.Get(*args.StartingConfiguration)->lock();

      if (vertex->Quality.has_value()) {
        switch (vertex->Quality.value()) {
          case WinState::Lose:
            std::cout << "Lost" << std::endl;
            break;
          case WinState::Draw:
            std::cout << "Draw" << std::endl;
            break;
          case WinState::Win:
            std::cout << "Won" << std::endl;
            break;
        }
      } else {
        std::cout << "Unkown" << std::endl;
      }

      break;
    }
  }

  if (args.ExportPaths)
    graph.Export(args.ExportPaths->first, args.ExportPaths->second);

  if (args.ImagesPath) graph.ExportImages(args.ImagesPath.value());
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
    const std::optional<std::filesystem::path> nodesPath =
        Parse::ParsePath(stream);
    if (!nodesPath) return false;

    const std::optional<std::filesystem::path> edgesPath =
        Parse::ParsePath(stream);
    if (!edgesPath) return false;

    ExportPaths = {nodesPath.value(), edgesPath.value()};

  } else if (argument == "--import" || argument == "-i") {
    const std::optional<std::filesystem::path> nodesPath =
        Parse::ParsePath(stream);
    if (!nodesPath) return false;

    const std::optional<std::filesystem::path> edgesPath =
        Parse::ParsePath(stream);
    if (!edgesPath) return false;

    ImportPaths = {nodesPath.value(), edgesPath.value()};

  } else if (argument == "--strategy") {
    const std::optional<StateGraphType> type = ParseStateGraphType(stream);
    if (!type) return false;

    Type = type.value();

  } else if (argument == "--image") {
    const std::optional<std::filesystem::path> imagesPath =
        Parse::ParsePath(stream);
    if (!imagesPath) return false;

    ImagesPath = imagesPath.value();

  } else {
    Parse::Unparse(stream, argument);
    return true;
  }

  return Parse(stream);
}

std::optional<StateGraphType> StateGraphArgs::ParseStateGraphType(
    std::istringstream& stream) {
  std::string string;
  stream >> string;

  if (string == "forward" || string == "forward-retrograde" ||
      string == "forward-retrograde-analysis") {
    return StateGraphType::ForwardRetrogradeAnalysis;
  } else if (string == "component") {
    return StateGraphType::Component;
  } else if (string == "retrograde" || string == "retrograde-analysis") {
    return StateGraphType::RetrogradeAnalysis;
  }

  std::cerr << std::format("Unknown state graph strategy \"{}\"!\n", string)
            << "Valid strategies are:\n"
               "- component\n"
               "- retrograde-analysis\n"
               "- forward-retrograde-analysis\n"
            << std::endl;
  return std::nullopt;
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
