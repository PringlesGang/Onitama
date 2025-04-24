#include "strategies.h"

namespace StateGraph {
namespace Strategies {

static void ExploreComponentRecursive(std::shared_ptr<Vertex> vertex,
                                      std::unordered_set<Game::Game>& exploring,
                                      Graph& graph) {
  const Game::Game game = Game::Game::FromSerialization(vertex->Serialization);
  if (exploring.contains(game)) return;
  exploring.insert(game);

  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextState = game;
    nextState.DoMove(move);

    const std::shared_ptr<Vertex> nextVertex =
        graph.Vertices.emplace(nextState, std::make_shared<Vertex>(nextState))
            .first->second;

    vertex->Edges.emplace_back(
        std::make_shared<Edge>(vertex, nextVertex, move));

    ExploreComponentRecursive(nextVertex, exploring, graph);
  }
}

void ExploreComponent(Graph& graph, Game::Game game) {
  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  const std::shared_ptr<Vertex> vertex =
      graph.Vertices.emplace(game, std::make_shared<Vertex>(game))
          .first->second;

  std::unordered_set<Game::Game> exploring;
  ExploreComponentRecursive(vertex, exploring, graph);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();
  std::cout << std::format("Run time: {}s", runTime) << std::endl;
}

}  // namespace Strategies
}  // namespace StateGraph
