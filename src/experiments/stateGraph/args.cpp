#include "args.h"

#include "../../stateGraph/stateGraph.h"
#include "../../stateGraph/strategies.h"

namespace Experiments {
namespace StateGraph {

using namespace ::StateGraph;
using namespace ::StateGraph::Strategies;

std::optional<StateGraphType> StateGraphArgs::ParseStateGraphType(
    std::istringstream& stream) {
  std::string string;
  stream >> string;

  if (string == "retrograde" || string == "retrograde-analysis") {
    return StateGraphType::RetrogradeAnalysis;
  } else if (string == "forward" || string == "forward-retrograde" ||
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
               "- retrograde-analysis\n"
               "- forward-retrograde-analysis\n"
               "- dispersed-frontier\n"
            << std::endl;
  return std::nullopt;
}

std::optional<std::shared_ptr<StateGraphArgs>> StateGraphArgs::Parse(
    std::istringstream& stream) {
  std::shared_ptr<StateGraphArgs> args = nullptr;

  // Parse strategy
  std::string strategy;
  stream >> strategy;

  if (strategy == "component") {
    args = std::make_shared<ComponentArgs>();
    if (!std::static_pointer_cast<ComponentArgs>(args)->Parse(stream))
      return std::nullopt;

  } else if (strategy == "retrograde" || strategy == "retrograde-analysis") {
    args = std::make_shared<RetrogradeAnalysisArgs>();
    if (!std::static_pointer_cast<RetrogradeAnalysisArgs>(args)->Parse(stream))
      return std::nullopt;

  } else if (strategy == "forward" || strategy == "forward-retrograde" ||
             strategy == "forward-retrograde-analysis") {
    args = std::make_shared<ForwardRetrogradeAnalysisArgs>();
    if (!std::static_pointer_cast<ForwardRetrogradeAnalysisArgs>(args)->Parse(
            stream)) {
      return std::nullopt;
    }

  } else if (strategy == "dispersed" || strategy == "dispersed-frontier") {
    args = std::make_shared<DispersedFrontierArgs>();
    if (!std::static_pointer_cast<DispersedFrontierArgs>(args)->Parse(stream))
      return std::nullopt;
  }

  if (args == nullptr) {
    std::cerr << "Failed to parse strategy type!" << std::endl;
    return std::nullopt;
  }

  // Parse initial state
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
    std::cerr << "Failed to parse initial state type!" << std::endl;
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

  } else if (parameter == "--intermediate") {
    IntermediateParameters = SaveParameters::Parse(stream);
    if (!IntermediateParameters) return false;

  } else if (parameter == "--load") {
    LoadPath = Parse::ParsePath(stream);
    if (!LoadPath) return false;

  } else {
    Parse::Unparse(stream, parameter);
    return true;
  }

  return ParseCommonArgs(stream);
}

Graph StateGraphArgs::GetGraph() {
  if (LoadPath) {
    const auto [graph, runtime] = Graph::Load(LoadPath.value());

    if (IntermediateParameters)
      IntermediateParameters->RuntimeTimer.Set(runtime, true);

    return graph;
  }

  if (ImportPaths)
    return Graph::Import(ImportPaths->first, ImportPaths->second);

  return Graph();
}

void ComponentArgs::Execute() {
  std::cout << "Generating state graph for:\n"
            << *StartingConfiguration << std::endl;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ExploreComponent(graph, *StartingConfiguration, IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  std::cout << std::format("Run time: {}s\n", runTime)
            << std::format("Analysed {} nodes and {} edges",
                           graph.GetNodeCount(), graph.GetEdgeCount())
            << std::endl;

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

void RetrogradeAnalysisArgs::Execute() {
  std::cout << "Finding perfect positional strategy for:\n"
            << *StartingConfiguration << std::endl;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ExploreComponent(graph, *StartingConfiguration, IntermediateParameters);
  RetrogradeAnalyse(graph);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
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
    std::cout << "Unknown" << std::endl;
  }

  std::cout << std::format("Run time: {}s\n", runTime)
            << std::format("Analysed {} nodes and {} edges",
                           graph.GetNodeCount(), graph.GetEdgeCount())
            << std::endl;

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

void ForwardRetrogradeAnalysisArgs::Execute() {
  std::cout << "Finding perfect positional strategy for:\n"
            << *StartingConfiguration << std::endl;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ForwardRetrogradeAnalysis(graph, *StartingConfiguration,
                            IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
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
    std::cout << "Unknown" << std::endl;
  }

  std::cout << std::format("Run time: {}s\n", runTime)
            << std::format("Analysed {} nodes and {} edges",
                           graph.GetNodeCount(), graph.GetEdgeCount())
            << std::endl;

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

void DispersedFrontierArgs::Execute() {
  std::cout << "Finding perfect positional strategy for:\n"
            << *StartingConfiguration << std::endl;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  DispersedFrontier(graph, *StartingConfiguration, Depth, MaxThreadCount,
                    IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
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
    std::cout << "Unknown" << std::endl;
  }

  std::cout << std::format("Run time: {}s\n", runTime)
            << std::format("Analysed {} nodes and {} edges",
                           graph.GetNodeCount(), graph.GetEdgeCount())
            << std::endl;

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

}  // namespace StateGraph
}  // namespace Experiments
