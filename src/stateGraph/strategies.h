#pragma once

#include "stateGraph.h"

namespace StateGraph {
namespace Strategies {

void RetrogradeAnalyse(Graph& graph);

void ExploreComponent(Graph& graph, Game::Game root);

void ForwardRetrogradeAnalysis(Graph& graph, Game::Game root);

void DispersedFrontier(Graph& graph, Game::Game root, size_t frontier,
                       size_t maxThreadCount);

}  // namespace Strategies
}  // namespace StateGraph
