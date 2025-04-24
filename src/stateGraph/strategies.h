#pragma once

#include "stateGraph.h"

namespace StateGraph {
namespace Strategies {

void RetrogradeAnalyse(Graph& graph);

void ExploreComponent(Graph& graph, const Game::Game root);

void ForwardRetrogradeAnalysis(Graph& graph, const Game::Game root);

}  // namespace Strategies
}  // namespace StateGraph
