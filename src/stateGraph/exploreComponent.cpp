#include "stateGraph.h"

namespace StateGraph {

void Graph::ExploreComponentRecursive(
    std::weak_ptr<Vertex> weakVertex,
    std::unordered_set<Game::Game>& exploring) {
  std::shared_ptr<Vertex> vertex = weakVertex.lock();
  if (vertex == nullptr) return;

  Game::Game game = Game::Game::FromSerialization(vertex->Serialization);

  if (exploring.contains(game)) return;
  exploring.insert(game);

  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextState = game;
    nextState.DoMove(move);

    if (!Vertices.contains(nextState))
      Vertices.insert({nextState, std::make_shared<Vertex>(nextState)});
    std::weak_ptr<Vertex> nextVertex = Vertices.at(nextState);

    vertex->Edges.emplace_back(
        std::make_shared<Edge>(vertex, nextVertex, move));
    ExploreComponentRecursive(nextVertex, exploring);
  }
}

std::weak_ptr<const Vertex> Graph::ExploreComponent(Game::Game&& game) {
  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  const std::shared_ptr<Vertex> vertex =
      Vertices.contains(game) ? Vertices.at(game)
                              : std::make_shared<Vertex>(std::move(game));

  std::unordered_set<Game::Game> exploring;
  ExploreComponentRecursive(vertex, exploring);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();
  std::cout << std::format("Run time: {}s", runTime) << std::endl;

  return vertex;
}

}  // namespace StateGraph
