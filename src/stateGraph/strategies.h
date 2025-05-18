#pragma once

#include "saveSystem.h"
#include "stateGraph.h"

namespace StateGraph {
namespace Strategies {

using namespace SaveSystem;

void RetrogradeAnalyse(std::shared_ptr<Vertex> source, WinState targetQuality,
                       std::shared_ptr<Edge> edge);
void RetrogradeAnalyse(Graph& graph);

void ExploreComponent(
    Graph& graph, Game::Game root, size_t maxDepth,
    std::optional<SaveParameters> saveParameters = std::nullopt);

void ForwardRetrogradeAnalysis(
    Graph& graph, Game::Game root,
    std::optional<SaveParameters> saveParameters = std::nullopt);

void DispersedFrontier(
    Graph& graph, Game::Game root, size_t frontier, size_t maxThreadCount,
    std::optional<SaveParameters> saveParameters = std::nullopt);

}  // namespace Strategies
}  // namespace StateGraph
