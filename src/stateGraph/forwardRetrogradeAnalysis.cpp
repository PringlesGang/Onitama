#include <cassert>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {

static void InsertUnique(std::shared_ptr<Edge> edge,
                         std::vector<std::shared_ptr<Edge>>& edges) {
  if (std::none_of(edges.begin(), edges.end(),
                   [edge](std::shared_ptr<Edge> otherEdge) {
                     return otherEdge->Move == edge->Move;
                   })) {
    edges.emplace_back(std::move(edge));
  }
}

static std::optional<WinState> Expand(
    const std::shared_ptr<Vertex> vertex, Graph& graph,
    std::unordered_set<std::shared_ptr<Vertex>>& expandingVertices,
    std::optional<SaveParameters>& saveParameters) {
  // Terminal state
  if (vertex->Quality.has_value()) {
    RetrogradeAnalyse(graph);

    if (saveParameters && saveParameters->ShouldSave())
      saveParameters->Save(graph);

    return vertex->Quality;
  }

  expandingVertices.insert(vertex);

  // Insert edges
  const Game::Game game = Game::Game::FromSerialization(vertex->Serialization);
  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextGame(game);
    nextGame.DoMove(move);

    const std::shared_ptr<Vertex> nextVertex =
        graph.Vertices.emplace(nextGame, std::make_shared<Vertex>(nextGame))
            .first->second;

    InsertUnique(std::make_shared<Edge>(vertex, nextVertex, move),
                 vertex->Edges);
  }
  assert(!game.HasValidMoves() ||
         vertex->Edges.size() == game.GetValidMoves().size());

  for (std::shared_ptr<Edge> edge : vertex->Edges) {
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    assert(target != nullptr);

    // Try to expand node
    if (!target->Quality.has_value()) {
      // Don't expand already expanding vertices
      if (expandingVertices.contains(target)) continue;

      const std::optional<WinState> targetQuality =
          Expand(target, graph, expandingVertices, saveParameters);

      // If the vertex has been coloured by retrograde analysis, then
      // early-exit
      if (vertex->Quality.has_value()) return vertex->Quality;
    }
  }

  return vertex->Quality;
}

void ForwardRetrogradeAnalysis(Graph& graph, const Game::Game root,
                               std::optional<SaveParameters> saveParameters) {
  const std::chrono::time_point startTime = std::chrono::system_clock::now();

  const std::shared_ptr<Vertex> rootVertex =
      graph.Vertices.emplace(root, std::make_shared<Vertex>(root))
          .first->second;

  if (rootVertex->Quality.has_value()) return;

  if (saveParameters) saveParameters->StartTimers();

  std::unordered_set<std::shared_ptr<Vertex>> expandingVertices;

  Expand(rootVertex, graph, expandingVertices, saveParameters);
  if (!rootVertex->Quality.has_value()) RetrogradeAnalyse(graph);

  const size_t runTime = std::chrono::duration_cast<std::chrono::seconds>(
                             std::chrono::system_clock::now() - startTime)
                             .count();
  std::cout << std::format("Run time: {}s", runTime) << std::endl;
}

}  // namespace Strategies
}  // namespace StateGraph
