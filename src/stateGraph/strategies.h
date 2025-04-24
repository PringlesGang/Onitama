#pragma once

#include "stateGraph.h"

namespace StateGraph {
namespace Strategies {

void RetrogradeAnalyse(Graph& graph);

void ForwardRetrogradeAnalysis(Graph& graph, const Game::Game root);

}  // namespace Strategies
}  // namespace StateGraph
