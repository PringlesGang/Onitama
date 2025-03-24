#include "stateGraph.h"

namespace StateGraph {

std::weak_ptr<const Vertex> Graph::FindPerfectStrategy(Game::Game&& game) {
  std::unordered_set<std::shared_ptr<Vertex>> draws;
  const std::weak_ptr<const Vertex> requestedVertex =
      FindPerfectStrategyExpand(std::move(game), draws);

  while (!draws.empty()) {
    std::unordered_set<std::shared_ptr<Vertex>> component;
    FindPerfectStrategyCheckDraw(*draws.begin(), component);

    for (const std::shared_ptr<Vertex> vertex : component) {
      if (!vertex->Quality.has_value()) vertex->Quality = WinState::Draw;
      draws.erase(vertex);
    }
  }

  return requestedVertex;
}

std::weak_ptr<Vertex> Graph::FindPerfectStrategyExpand(
    Game::Game&& game, std::unordered_set<std::shared_ptr<Vertex>>& draws) {
  if (Vertices.contains(game)) return Vertices.at(std::move(game));

  std::shared_ptr<Vertex> info = std::make_shared<Vertex>(game);
  Vertices.emplace(game, info);

  // Terminal game state
  if (info->Quality.has_value()) return info;

  const std::vector<Game::Move>& validMoves = game.GetValidMoves();
  for (const Game::Move move : validMoves) {
    Game::Game nextState(game);
    nextState.DoMove(move);

    // Traverse further
    std::shared_ptr<Vertex> nextInfo =
        FindPerfectStrategyExpand(std::move(nextState), draws).lock();
    if (nextInfo == nullptr) continue;
    info->Edges.emplace(move, nextInfo);

    const std::optional<WinState> nextQuality =
        nextInfo->Quality.transform(operator-);

    // Select the best move yet
    if (!info->Quality || info->Quality < nextQuality) {
      info->Quality = nextQuality;
      info->OptimalMove = move;
    }

    // A winning positional strategy has been found
    if (info->Quality == WinState::Win) return info;
  }

  if (!info->Quality) draws.insert(info);

  return info;
}

void Graph::FindPerfectStrategyCheckDraw(
    std::weak_ptr<Vertex> weakVertex,
    std::unordered_set<std::shared_ptr<Vertex>>& component) {
  // Check if the vertex has already been checked
  const std::shared_ptr<Vertex> vertex = weakVertex.lock();
  if (vertex == nullptr || component.contains(vertex)) return;
  component.insert(vertex);

  // Check if a winning move has popped up
  for (const auto [move, nextVertexWeak] : vertex->Edges) {
    const std::shared_ptr<const Vertex> nextVertex = nextVertexWeak.lock();
    if (nextVertex != nullptr && nextVertex->Quality == WinState::Lose) {
      vertex->Quality = WinState::Win;
      vertex->OptimalMove = move;
      return;
    }
  }

  // Find the next best move
  bool allLosingMoves = true;
  for (auto edgeIt = vertex->Edges.begin(); edgeIt != vertex->Edges.end();
       edgeIt++) {
    const auto [move, nextVertexWeak] = *edgeIt;

    const std::shared_ptr<Vertex> nextVertex = nextVertexWeak.lock();
    if (nextVertex == nullptr || nextVertex->Quality.has_value()) continue;

    FindPerfectStrategyCheckDraw(nextVertex, component);

    switch (nextVertex->Quality.value_or(WinState::Draw)) {
      case WinState::Lose: {
        vertex->Quality = WinState::Win;
        vertex->OptimalMove = move;

        // Recheck the previously searched draw edges, now that the loop is
        // broken
        for (auto checkedEdges = vertex->Edges.begin(); checkedEdges != edgeIt;
             checkedEdges++) {
          std::shared_ptr<Vertex> checkedVertex = checkedEdges->second.lock();
          if (checkedVertex == nullptr || nextVertex->Quality.has_value())
            continue;

          std::unordered_set<std::shared_ptr<Vertex>> newComponent({vertex});
          FindPerfectStrategyCheckDraw(std::move(checkedVertex), newComponent);
        }

        return;
      }
      case WinState::Draw: {
        vertex->OptimalMove = move;
        allLosingMoves = false;

        break;
      }
      case WinState::Win: {
        break;
      }
    }
  }

  if (allLosingMoves) vertex->Quality = WinState::Lose;
}

}  // namespace StateGraph
