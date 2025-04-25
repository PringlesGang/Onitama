#include "strategies.h"

namespace StateGraph {
namespace Strategies {

static void ExploreComponentRecursive(
    std::shared_ptr<Vertex> vertex, std::unordered_set<Game::Game>& exploring,
    Graph& graph, std::optional<SaveParameters>& saveParameters) {
  const Game::Game game = Game::Game::FromSerialization(vertex->Serialization);
  exploring.insert(game);

  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextState = game;
    nextState.DoMove(move);

    // Don't expand already expanded vertices
    if (exploring.contains(nextState)) continue;

    const std::shared_ptr<Vertex> nextVertex =
        graph.Vertices.emplace(nextState, std::make_shared<Vertex>(nextState))
            .first->second;

    vertex->Edges.emplace_back(
        std::make_shared<Edge>(vertex, nextVertex, move));

    ExploreComponentRecursive(nextVertex, exploring, graph, saveParameters);
  }
}

void ExploreComponent(Graph& graph, Game::Game game,
                      std::optional<SaveParameters> saveParameters) {
  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  const std::shared_ptr<Vertex> vertex =
      graph.Vertices.emplace(game, std::make_shared<Vertex>(game))
          .first->second;

  std::unordered_set<Game::Game> exploring;
  ExploreComponentRecursive(vertex, exploring, graph, saveParameters);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();
  std::cout << std::format("Run time: {}s", runTime) << std::endl;
}

}  // namespace Strategies
}  // namespace StateGraph
