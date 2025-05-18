#include <cassert>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {

static void ExploreComponentRecursive(
    std::shared_ptr<Vertex> vertex,
    std::unordered_set<Game::Game, Hash, EqualTo>& exploring,
    std::unordered_set<std::shared_ptr<Vertex>>& frontier, const size_t depth,
    const size_t maxDepth, Graph& graph,
    std::optional<SaveParameters>& saveParameters) {
  const Game::Game game = Game::Game::FromSerialization(vertex->Serialization);
  exploring.insert(game);
  frontier.erase(vertex);

  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextState = game;
    nextState.DoMove(move);

    // Don't expand already expanded vertices
    if (exploring.contains(nextState)) continue;

    const std::shared_ptr<Vertex> nextVertex =
        graph.Vertices.emplace(nextState, std::make_shared<Vertex>(nextState))
            .first->second;

    assert(Game::Game::FromSerialization(nextVertex->Serialization) ==
           nextState);

    vertex->Edges.emplace_back(
        std::make_shared<Edge>(vertex, nextVertex, move));

    if (maxDepth == 0 || depth + 1 <= maxDepth) {
      ExploreComponentRecursive(nextVertex, exploring, frontier, depth + 1,
                                maxDepth, graph, saveParameters);
    } else {
      frontier.insert(nextVertex);
    }
  }
}

void ExploreComponent(Graph& graph, Game::Game game, const size_t maxDepth,
                      std::optional<SaveParameters> saveParameters) {
  const std::shared_ptr<Vertex> root =
      graph.Vertices.emplace(game, std::make_shared<Vertex>(game))
          .first->second;

  std::unordered_set<Game::Game, Hash, EqualTo> exploring;
  std::unordered_set<std::shared_ptr<Vertex>> frontier = {root};

  while (!frontier.empty()) {
    const std::shared_ptr<Vertex> vertex = *frontier.begin();
    ExploreComponentRecursive(vertex, exploring, frontier, 0, maxDepth, graph,
                              saveParameters);
  }
}

}  // namespace Strategies
}  // namespace StateGraph
