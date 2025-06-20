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

  } else if (parameter == "--disable-symmetries") {
    UseSymmetries = false;

  } else if (parameter == "--data") {
    Data = true;

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

bool ComponentArgs::Parse(std::istringstream& stream) {
  if (!(stream >> MaxDepth)) {
    std::cerr << "Failed to parse Component's max depth!" << std::endl;
    return false;
  }

  return true;
}

void ComponentArgs::Execute() {
  if (!Data) {
    std::cout << "Generating state graph for:\n"
              << *StartingConfiguration << std::endl;
  }

  StateGraph::UseSymmetries = UseSymmetries;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ExploreComponent(graph, *StartingConfiguration, MaxDepth,
                   IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  if (!Data) {
    std::cout << std::format("Run time: {:.3f}s\n", runTime / 1000.0f)
              << std::format("Analysed {} nodes and {} edges",
                             graph.GetNodeCount(), graph.GetEdgeCount())
              << std::endl;
  } else {
    std::cout << std::format("{:.3f},{},{}", runTime / 1000.0f,
                             graph.GetNodeCount(), graph.GetEdgeCount())
              << std::endl;
  }

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

bool RetrogradeAnalysisArgs::Parse(std::istringstream& stream) {
  if (!(stream >> MaxDepth)) {
    std::cerr << "Failed to parse Retrograde Analysis's max depth!"
              << std::endl;
    return false;
  }

  return true;
}

void RetrogradeAnalysisArgs::Execute() {
  if (!Data) {
    std::cout << "Finding perfect positional strategy for:\n"
              << *StartingConfiguration << std::endl;
  }

  StateGraph::UseSymmetries = UseSymmetries;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ExploreComponent(graph, *StartingConfiguration, MaxDepth,
                   IntermediateParameters);

  const std::chrono::time_point finishedExploringTime =
      std::chrono::system_clock::now();

  RetrogradeAnalyse(graph);

  const std::chrono::time_point finishedRetrogradeTime =
      std::chrono::system_clock::now();

  const size_t exploreTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          finishedExploringTime - startTime)
          .count();
  const size_t retrogradeTime =
      std::chrono::duration_cast<std::chrono::milliseconds>(
          finishedRetrogradeTime - finishedExploringTime)
          .count();
  const size_t runTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             finishedRetrogradeTime - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
      graph.Get(*StartingConfiguration)->lock();
  std::string qualityString = "Unknown";
  if (vertex->Quality.has_value()) {
    switch (vertex->Quality.value()) {
      case WinState::Lose:
        qualityString = "Lost";
        break;
      case WinState::Draw:
        qualityString = "Draw";
        break;
      case WinState::Win:
        qualityString = "Won";
        break;
    }
  }

  if (Data) {
    std::cout << std::format("{:.3f},{:.3f},{:.3f},{},{},{}",
                             exploreTime / 1000.0f, retrogradeTime / 1000.0f,
                             runTime / 1000.0f, graph.GetNodeCount(),
                             graph.GetEdgeCount(), qualityString)
              << std::endl;

  } else {
    std::cout << qualityString << "\n"
              << std::format("Graph exploration time: {:.3f}s\n",
                             exploreTime / 1000.0f)
              << std::format("Retrograde analysis time: {:.3f}s\n",
                             retrogradeTime / 1000.0f)
              << std::format("Total run time: {:.3f}s\n", runTime / 1000.0f)
              << std::format("Analysed {} nodes and {} edges",
                             graph.GetNodeCount(), graph.GetEdgeCount())
              << std::endl;
  }

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

void ForwardRetrogradeAnalysisArgs::Execute() {
  if (!Data) {
    std::cout << "Finding perfect positional strategy for:\n"
              << *StartingConfiguration << std::endl;
  }

  StateGraph::UseSymmetries = UseSymmetries;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  ForwardRetrogradeAnalysis(graph, *StartingConfiguration,
                            IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
      graph.Get(*StartingConfiguration)->lock();
  std::string qualityString = "Unknown";
  if (vertex->Quality.has_value()) {
    switch (vertex->Quality.value()) {
      case WinState::Lose:
        qualityString = "Lost";
        break;
      case WinState::Draw:
        qualityString = "Draw";
        break;
      case WinState::Win:
        qualityString = "Won";
        break;
    }
  }

  if (Data) {
    std::cout << std::format("{:.3f},{},{},{}", runTime / 1000.0f,
                             graph.GetNodeCount(), graph.GetEdgeCount(),
                             qualityString)
              << std::endl;

  } else {
    std::cout << qualityString << "\n"
              << std::format("Run time: {:.3f}s\n", runTime / 1000.0f)
              << std::format("Analysed {} nodes and {} edges",
                             graph.GetNodeCount(), graph.GetEdgeCount())
              << std::endl;
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

void DispersedFrontierArgs::Execute() {
  if (!Data) {
    std::cout << "Finding perfect positional strategy for:\n"
              << *StartingConfiguration << std::endl;
  }

  StateGraph::UseSymmetries = UseSymmetries;

  Graph graph = GetGraph();

  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  DispersedFrontier(graph, *StartingConfiguration, Depth, MaxThreadCount,
                    IntermediateParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();

  const std::shared_ptr<const Vertex> vertex =
      graph.Get(*StartingConfiguration)->lock();
  std::string qualityString = "Unknown";
  if (vertex->Quality.has_value()) {
    switch (vertex->Quality.value()) {
      case WinState::Lose:
        qualityString = "Lost";
        break;
      case WinState::Draw:
        qualityString = "Draw";
        break;
      case WinState::Win:
        qualityString = "Won";
        break;
    }
  }

  if (Data) {
    std::cout << std::format("{:.3f},{},{},{}", runTime / 1000.0f,
                             graph.GetNodeCount(), graph.GetEdgeCount(),
                             qualityString)
              << std::endl;

  } else {
    std::cout << qualityString << "\n"
              << std::format("Run time: {:.3f}s\n", runTime / 1000.0f)
              << std::format("Analysed {} nodes and {} edges",
                             graph.GetNodeCount(), graph.GetEdgeCount())
              << std::endl;
  }

  if (ExportPaths) graph.Export(ExportPaths->first, ExportPaths->second);
  if (ImagesPath) graph.ExportImages(ImagesPath.value());
}

}  // namespace StateGraph
}  // namespace Experiments
