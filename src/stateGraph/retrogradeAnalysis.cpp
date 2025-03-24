#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::RetrogradeAnalysis(Game::Game&& game) {
  if (Vertices.contains(game)) {
    const std::shared_ptr<const Vertex> vertex = Vertices.at(game);
    if (vertex->Quality != WinState::Draw) return vertex;
  }
}

}  // namespace StateGraph
