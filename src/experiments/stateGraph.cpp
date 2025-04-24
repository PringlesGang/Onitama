#include "stateGraph.h"

#include <format>
#include <iostream>

#include "../cli/game.h"
#include "../stateGraph/stateGraph.h"
#include "../stateGraph/strategies.h"
#include "../util/base64.h"
#include "../util/parse.h"

namespace Experiments {
namespace StateGraph {

std::optional<StateGraphType> StateGraphArgs::ParseStateGraphType(
    std::istringstream& stream) {
  std::string string;
  stream >> string;

  if (string == "forward" || string == "forward-retrograde" ||
      string == "forward-retrograde-analysis") {
    return StateGraphType::ForwardRetrogradeAnalysis;
  } else if (string == "component") {
    return StateGraphType::Component;
  } else if (string == "dispersed" || string == "dispersed-frontier") {
    return StateGraphType::DispersedFrontier;
  }

  std::cerr << std::format("Unknown state graph strategy \"{}\"!\n", string)
            << "Valid strategies are:\n"
               "- component\n"
               "- forward-retrograde-analysis\n"
               "- dispersed-frontier\n"
            << std::endl;
  return std::nullopt;
}

std::optional<std::shared_ptr<StateGraphArgs>> StateGraphArgs::Parse(
    std::istringstream& stream) {
  std::shared_ptr<StateGraphArgs> args;

  std::string stateType;
  stream >> stateType;

  if (stateType == "game") {
    Parse::GameConfiguration config;
    if (!config.Parse(stream) || !config.IsValid()) return std::nullopt;
    args->StartingConfiguration =
        std::make_shared<Game::Game>(config.ToGame().value());

  } else if (stateType == "state") {
    const std::optional<Game::GameSerialization> serialization =
        Game::Game::ParseSerialization(stream);

    if (!serialization) return std::nullopt;

    args->StartingConfiguration = std::make_shared<Game::Game>(
        std::move(Game::Game::FromSerialization(serialization.value())));

  } else {
    std::cerr << "Failed to parse starting state type!" << std::endl;
    return std::nullopt;
  }

  if (!args->ParseCommonArgs(stream)) return std::nullopt;

  std::string parameter;
  stream >> parameter;

  if (parameter == "--strategy") {
    std::string strategy;
    stream >> strategy;

    if (strategy == "component") {
      args = std::make_shared<ComponentArgs>();
      if (!args->Parse(stream)) return std::nullopt;

    } else if (strategy == "forward" || strategy == "forward-retrograde" ||
               strategy == "forward-retrograde-analysis") {
      args = std::make_shared<ForwardRetrogradeAnalysisArgs>();
      if (!args->Parse(stream)) return std::nullopt;

    } else if (strategy == "dispersed" || strategy == "dispersed-frontier") {
      args = std::make_shared<DispersedFrontierArgs>();
      if (!args->Parse(stream)) return std::nullopt;

    } else {
      std::cerr << std::format("Unknown strategy \"{}\"!", strategy)
                << std::endl;
      return std::nullopt;
    }

  } else {
    std::cerr << std::format("Unknown parameter \"{}\"!", parameter)
              << std::endl;
    return std::nullopt;
  }

  if (!args->ParseCommonArgs(stream)) return std::nullopt;
  if (!Parse::Terminate(stream)) return std::nullopt;

  return args;
}

bool StateGraphArgs::ParseCommonArgs(std::istringstream& stream) {
  std::string parameter;
  stream >> parameter;

  if (parameter == "--export" || parameter == "-e") {
    const std::optional<std::filesystem::path> nodesPath =
        Parse::ParsePath(stream);
    if (!nodesPath) return false;

    const std::optional<std::filesystem::path> edgesPath =
        Parse::ParsePath(stream);
    if (!edgesPath) return false;

    ExportPaths = {std::move(nodesPath.value()), std::move(edgesPath.value())};

  } else if (parameter == "--import" || parameter == "-i") {
    const std::optional<std::filesystem::path> nodesPath =
        Parse::ParsePath(stream);
    if (!nodesPath) return false;

    const std::optional<std::filesystem::path> edgesPath =
        Parse::ParsePath(stream);
    if (!edgesPath) return false;

    ExportPaths = {std::move(nodesPath.value()), std::move(edgesPath.value())};

  } else if (parameter == "--images") {
    ImagesPath = Parse::ParsePath(stream);
    if (!ImagesPath) return false;

  } else {
    Parse::Unparse(stream, parameter);
    return true;
  }

  return ParseCommonArgs(stream);
}

void ComponentArgs::Execute() const {
  std::cout << "Generating state graph for:\n"
            << *StartingConfiguration << std::endl;

  ::StateGraph::Graph graph =
      ImportPaths
          ? ::StateGraph::Graph::Import(ImportPaths->first, ImportPaths->second)
          : ::StateGraph::Graph();

  graph.ExploreComponent(Game::Game(*StartingConfiguration));

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

bool ForwardRetrogradeAnalysisArgs::Parse(std::istringstream& stream) {
  std::string parameter;
  stream >> parameter;

  if (parameter == "--intermediate") {
    IntermediatePath = Parse::ParsePath(stream);
    if (!IntermediatePath) return false;

    if (!(stream >> SaveTimeInterval)) {
      std::cerr << "Failed to parse save time interval!" << std::endl;
      return false;
    }

  } else {
    Parse::Unparse(stream, parameter);
    return true;
  }

  return Parse(stream);
}

void ForwardRetrogradeAnalysisArgs::Execute() const {
  std::cout << "Finding perfect positional strategy for:\n"
            << *StartingConfiguration << std::endl;

  ::StateGraph::Graph graph =
      ImportPaths
          ? ::StateGraph::Graph::Import(ImportPaths->first, ImportPaths->second)
          : ::StateGraph::Graph();

  ::StateGraph::Strategies::ForwardRetrogradeAnalysis(graph,
                                                      *StartingConfiguration);

  const std::shared_ptr<const ::StateGraph::Vertex> vertex =
      graph.Get(*StartingConfiguration)->lock();
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

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

bool DispersedFrontierArgs::Parse(std::istringstream& stream) {
  if (!(stream >> Depth)) {
    std::cerr << "Failed to parse dispersed frontier depth!" << std::endl;
    return false;
  }

  if (Depth == 0) {
    std::cerr << "Dispersed frontier depth cannot be zero!" << std::endl;
    return false;
  }

  if (!(stream >> MaxThreadCount)) {
    std::cerr << "Failed to parse dispersed frontier max thread count!"
              << std::endl;
    return false;
  }

  if (MaxThreadCount == 0) {
    std::cerr << "Dispersed frontier thread count cannot be zero!" << std::endl;
    return false;
  }

  return true;
}

void DispersedFrontierArgs::Execute() const {
  std::cout << "Finding perfect positional strategy for:\n"
            << *StartingConfiguration << std::endl;

  ::StateGraph::Graph graph =
      ImportPaths
          ? ::StateGraph::Graph::Import(ImportPaths->first, ImportPaths->second)
          : ::StateGraph::Graph();

  graph.DispersedFrontier(Game::Game(*StartingConfiguration), Depth,
                          MaxThreadCount);

  const std::shared_ptr<const ::StateGraph::Vertex> vertex =
      graph.Get(*StartingConfiguration)->lock();
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

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

std::optional<Cli::Thunk> Parse(std::istringstream& command) {
  const std::optional<std::shared_ptr<StateGraphArgs>> args =
      StateGraphArgs::Parse(command);

  if (!args.has_value()) return std::nullopt;
  if (!args.value()->IsValid()) return std::nullopt;

  return [args] { args.value()->Execute(); };
}

}  // namespace StateGraph
}  // namespace Experiments
