#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::ForwardRetrogradeAnalysis(
    Game::Game&& game) {
  std::shared_ptr<Vertex> rootVertex = std::make_shared<Vertex>(game);
  Vertices.emplace(game, rootVertex);

  std::unordered_set<std::shared_ptr<Vertex>> expandedVertices;
  std::unordered_set<std::shared_ptr<Edge>> unlabelledEdges;
  ForwardRetrogradeAnalysisExpand(rootVertex, expandedVertices, unlabelledEdges,
                                  rootVertex);

  RetrogradeAnalyseEdges(unlabelledEdges);

  const auto isDraw = [&expandedVertices](std::shared_ptr<Edge> edge) -> bool {
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    if (target == nullptr) return false;

    return (target->Quality.value_or(WinState::Draw) == WinState::Draw) &&
           expandedVertices.contains(target);
  };

  // All edges between two expanded, non-labelled vertices result in draws
  for (const std::shared_ptr<Vertex> vertex : expandedVertices) {
    if (vertex->Quality.has_value()) continue;

    const auto drawMove =
        std::find_if(vertex->Edges.begin(), vertex->Edges.end(), isDraw);
    if (drawMove == vertex->Edges.end()) continue;

    vertex->SetOptimalMove((*drawMove)->Move);
    vertex->Quality = WinState::Draw;
  }

  return rootVertex;
}

void Graph::ForwardRetrogradeAnalysisExpand(
    const std::shared_ptr<Vertex> source,
    std::unordered_set<std::shared_ptr<Vertex>>& expandedVertices,
    std::unordered_set<std::shared_ptr<Edge>>& unlabelledEdges,
    const std::shared_ptr<const Vertex> root) {
  if (expandedVertices.contains(source)) return;
  expandedVertices.insert(source);

  const Game::Game game = Game::Game::FromSerialization(source->Serialization);

  // Terminal game state
  if (source->Quality.has_value()) {
    RetrogradeAnalyseEdges(unlabelledEdges);
    return;
  }

  // Insert edges
  const std::vector<Game::Move>& validMoves = game.GetValidMoves();
  for (const Game::Move move : validMoves) {
    Game::Game nextState(game);
    nextState.DoMove(move);

    if (!Vertices.contains(nextState))
      Vertices.emplace(nextState, std::make_shared<Vertex>(nextState));
    const std::shared_ptr<Vertex> target = Vertices.at(nextState);

    const std::shared_ptr<Edge> edge =
        std::make_shared<Edge>(source, target, move);
    unlabelledEdges.insert(edge);
    source->Edges.emplace_back(std::move(edge));
  }

  bool allLabelled = true;
  for (auto edgeIt = source->Edges.begin(); edgeIt != source->Edges.end();
       edgeIt++) {
    const std::shared_ptr<Edge> edge = *edgeIt;
    const bool finalEdge = edgeIt + 1 == source->Edges.end();

    std::shared_ptr<Vertex> target = edge->Target.lock();
    if (target == nullptr) {
      unlabelledEdges.erase(edge);
      continue;
    }

    ForwardRetrogradeAnalysisExpand(target, expandedVertices, unlabelledEdges,
                                    root);

    // Optimal strategy from root has been established; exit algorithm
    if (root->Quality.has_value()) {
      // Current vertex was not fully expanded
      expandedVertices.erase(source);
      return;
    }

    if (!target->Quality.has_value()) {
      allLabelled = false;
      continue;
    }

    switch (target->Quality.value()) {
      case WinState::Lose: {
        source->Quality = WinState::Win;
        source->SetOptimalMove(edge->Move);
        unlabelledEdges.erase(edge);

        return;
      }

      case WinState::Win: {
        if (finalEdge && allLabelled) {  // last unlabelled
          source->Quality = WinState::Lose;
          source->SetOptimalMove(edge->Move);
        } else {
          edge->Optimal = false;
        }

        unlabelledEdges.erase(edge);
        continue;
      }

      default: {
        const std::string msg = std::format("Unexpected target quality \"{}\"!",
                                            (int8_t)target->Quality.value());
        throw std::runtime_error(msg);
      }
    }
  }
}

}  // namespace StateGraph
