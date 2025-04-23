#include <cassert>
#include <chrono>
#include <future>
#include <unordered_set>

#include "stateGraph.h"

namespace StateGraph {

namespace DispersedFrontier {

struct VertexInfo {
  VertexInfo(Game::GameSerialization game) : Game(game) {}
  VertexInfo(Game::Game game) : Game(game.Serialize()) {}

  Game::GameSerialization Game;
  std::vector<Game::Move> Edges;
};

struct ThreadContext {
  std::unordered_map<Game::Game, VertexInfo, Hash, EqualTo> LocalVertices;
  std::unordered_set<Game::Game, Hash, EqualTo> Frontier;

  std::optional<std::future<void>> thread = std::nullopt;

  bool InUse() const { return thread.has_value(); }

  void Finish(std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash,
                                 EqualTo>& globalVertices,
              std::unordered_set<Game::Game, Hash, EqualTo>& globalFrontier);

  void Reset() {
    LocalVertices.clear();
    Frontier.clear();
    thread = std::nullopt;
  }
};

}  // namespace DispersedFrontier

using namespace std::chrono_literals;
using namespace DispersedFrontier;

static void Explore(
    const Game::Game game,
    std::unordered_map<Game::Game, VertexInfo, Hash, EqualTo>& localVertices,
    std::unordered_set<Game::Game, Hash, EqualTo>& frontier,
    const std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash,
                             EqualTo>& globalVertices,
    const size_t depth, const size_t maxDepth) {
  // Check frontier depth
  if (depth >= maxDepth) {
    frontier.emplace(game);
    return;
  }

  // Update local vertices
  VertexInfo& vertex =
      localVertices.emplace(game, VertexInfo(game)).first->second;

  // Analyse moves
  vertex.Edges = game.GetValidMoves();
  for (const Game::Move move : vertex.Edges) {
    Game::Game next(game);
    next.DoMove(move);

    // Don't explore already explored states
    if (globalVertices.contains(next) || localVertices.contains(next)) continue;

    // Venture forth
    Explore(std::move(next), localVertices, frontier, globalVertices, depth + 1,
            maxDepth);
  }
}

void ThreadContext::Finish(
    std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash, EqualTo>&
        globalVertices,
    std::unordered_set<Game::Game, Hash, EqualTo>& globalFrontier) {
  // Add found vertices
  for (const auto [game, localVertex] : LocalVertices) {
    const std::shared_ptr<Vertex> vertex =
        globalVertices.emplace(game, std::make_shared<Vertex>(game))
            .first->second;

    // Add edges
    assert(localVertex.Edges.size() == game.GetValidMoves().size());
    for (Game::Move move : localVertex.Edges) {
      Game::Game nextGame(game);
      nextGame.DoMove(move);

      const std::shared_ptr<Vertex> nextVertex =
          globalVertices.emplace(nextGame, std::make_shared<Vertex>(nextGame))
              .first->second;

      // Only add if not already there
      if (std::none_of(vertex->Edges.begin(), vertex->Edges.end(),
                       [move](const std::shared_ptr<Edge> edge) {
                         return edge->Move == move;
                       })) {
        vertex->Edges.emplace_back(
            std::make_shared<Edge>(vertex, nextVertex, move));
      }
    }
  }

  // Update frontier
  for (const Game::Game& frontierGame : Frontier) {
    std::shared_ptr<Vertex> vertex = globalVertices.at(frontierGame);
    const bool expanded = vertex->Quality.has_value() || !vertex->Edges.empty();

    if (expanded) {
      globalFrontier.erase(frontierGame);
    } else {
      globalFrontier.insert(frontierGame);
    }
  }

  Reset();
}

std::weak_ptr<const Vertex> Graph::DispersedFrontier(
    Game::Game&& game, const size_t depth, const size_t maxThreadCount) {
  std::unordered_set<Game::Game, Hash, EqualTo> frontier = {game};

  std::vector<ThreadContext> threadContexts(maxThreadCount);

  const auto anyThreadActive = [&threadContexts] {
    return std::any_of(
        threadContexts.begin(), threadContexts.end(),
        [](const ThreadContext& context) { return context.InUse(); });
  };

  do {
    if (frontier.empty()) {
      // Wait for the first thread to finish

      assert(anyThreadActive());
      for (ThreadContext& context : threadContexts) {
        if (!context.InUse()) continue;

        if (context.thread->wait_for(0ms) == std::future_status::ready) {
          context.Finish(Vertices, frontier);
          break;
        }
      }

      continue;
    }

    // Find an idle thread
    ThreadContext* idleContext = nullptr;
    do {
      for (ThreadContext& context : threadContexts) {
        // An unused thread is idling
        if (!context.InUse()) {
          idleContext = &context;
          break;
        }

        // A finished thread is idling
        if (context.thread->wait_for(0ms) == std::future_status::ready) {
          context.Finish(Vertices, frontier);
          idleContext = &context;
          break;
        }
      }
    } while (idleContext == nullptr);

    // Put the idle thread to work
    const Game::Game state = *frontier.begin();
    frontier.erase(state);

    idleContext->thread =
        std::async(std::launch::async, [state, idleContext, this, depth]() {
          Explore(std::move(state), idleContext->LocalVertices,
                  idleContext->Frontier, Vertices, 0, depth);
        });
  } while (!frontier.empty() || anyThreadActive());

  RetrogradeAnalyse(*this);
  return Vertices.at(game);
}

}  // namespace StateGraph
