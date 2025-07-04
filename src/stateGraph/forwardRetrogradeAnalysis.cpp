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
    const std::shared_ptr<const Vertex> root,
    std::optional<SaveParameters>& saveParameters) {
  expandingVertices.insert(vertex);

  // Insert edges
  const Game::Game game = Game::Game::FromSerialization(vertex->Serialization);
  bool terminalStateFound = false;
  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextGame(game);
    nextGame.DoMove(move);

    const auto [nextGameIt, inserted] =
        graph.Vertices.emplace(nextGame, std::make_shared<Vertex>(nextGame));
    const std::shared_ptr<Vertex> nextVertex = nextGameIt->second;

    InsertUnique(std::make_shared<Edge>(vertex, nextVertex, move),
                 vertex->Edges);

    if (inserted && nextVertex->Quality.has_value()) {
      // New terminal state found
      assert(nextVertex->Edges.empty() &&
             nextVertex->Quality.value() == WinState::Lose);

      expandingVertices.insert(nextVertex);
      terminalStateFound = true;
    }
  }

  if (terminalStateFound) {
    RetrogradeAnalyse(graph);

    if (saveParameters && saveParameters->ShouldSave())
      saveParameters->Save(graph);

    // Because there's a direct move to a terminal state, this state will have
    // to be labelled winning by retrograde analysis
    assert(vertex->Quality == WinState::Win);
    return vertex->Quality;
  }

  assert(!game.HasValidMoves() ||
         vertex->Edges.size() == game.GetValidMoves().size());

  for (std::shared_ptr<Edge> edge : vertex->Edges) {
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    assert(target != nullptr);

    // Try to expand node if not already being expanded
    if (!expandingVertices.contains(target)) {
      Expand(target, graph, expandingVertices, root, saveParameters);

      // If the current or root vertex has been coloured by retrograde analysis,
      // then early-exit
      if (vertex->Quality.has_value() || root->Quality.has_value()) {
        return vertex->Quality;
      }
    }

    if (target->Quality.has_value() && !edge->Optimal.has_value()) {
      RetrogradeAnalyse(vertex, target->Quality.value(), edge);
      if (vertex->Quality.has_value()) return vertex->Quality;
    }
  }

  return vertex->Quality;
}

void ForwardRetrogradeAnalysis(Graph& graph, const Game::Game root,
                               std::optional<SaveParameters> saveParameters) {
  const std::shared_ptr<Vertex> rootVertex =
      graph.Vertices.emplace(root, std::make_shared<Vertex>(root))
          .first->second;

  if (rootVertex->Quality.has_value()) return;

  if (saveParameters) saveParameters->StartTimers();

  std::unordered_set<std::shared_ptr<Vertex>> expandingVertices;

  Expand(rootVertex, graph, expandingVertices, rootVertex, saveParameters);
  if (!rootVertex->Quality.has_value()) RetrogradeAnalyse(graph);
}

}  // namespace Strategies
}  // namespace StateGraph
