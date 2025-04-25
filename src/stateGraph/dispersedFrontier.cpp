#include <cassert>
#include <chrono>
#include <future>
#include <shared_mutex>
#include <unordered_set>

#include "strategies.h"

namespace StateGraph {
namespace Strategies {
namespace DF {

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
              std::unordered_set<Game::Game, Hash, EqualTo>& globalFrontier,
              std::shared_mutex& mutex);

  void Reset() {
    LocalVertices.clear();
    Frontier.clear();
    thread = std::nullopt;
  }
};

}  // namespace DF

using namespace std::chrono_literals;
using namespace DF;

static void Explore(
    const Game::Game game,
    std::unordered_map<Game::Game, VertexInfo, Hash, EqualTo>& localVertices,
    std::unordered_set<Game::Game, Hash, EqualTo>& frontier,
    const std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash,
                             EqualTo>& globalVertices,
    const size_t depth, const size_t maxDepth, std::shared_mutex& mutex) {
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
    if (localVertices.contains(next)) continue;

    // If the global is locked, don't waste time and just keep going,
    // assuming it has not yet been explored
    if (mutex.try_lock_shared()) {
      const bool explored = globalVertices.contains(next);
      mutex.unlock_shared();

      if (explored) continue;
    }

    // Venture forth
    Explore(std::move(next), localVertices, frontier, globalVertices, depth + 1,
            maxDepth, mutex);
  }
}

void ThreadContext::Finish(
    std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash, EqualTo>&
        globalVertices,
    std::unordered_set<Game::Game, Hash, EqualTo>& globalFrontier,
    std::shared_mutex& mutex) {
  mutex.lock();

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

  mutex.unlock();
  Reset();
}

void DispersedFrontier(Graph& graph, const Game::Game game, const size_t depth,
                       const size_t maxThreadCount,
                       std::optional<SaveParameters> saveParameters) {
  std::unordered_set<Game::Game, Hash, EqualTo> frontier = {game};

  std::shared_mutex mutex;
  std::vector<ThreadContext> threadContexts(maxThreadCount);

  const auto anyThreadActive = [&threadContexts] {
    return std::any_of(
        threadContexts.begin(), threadContexts.end(),
        [](const ThreadContext& context) { return context.InUse(); });
  };

  if (saveParameters) saveParameters->StartTimers();

  do {
    if (frontier.empty()) {
      // Wait for the first thread to finish

      assert(anyThreadActive());
      for (ThreadContext& context : threadContexts) {
        if (!context.InUse()) continue;

        if (context.thread->wait_for(0ms) == std::future_status::ready) {
          context.Finish(graph.Vertices, frontier, mutex);
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
          context.Finish(graph.Vertices, frontier, mutex);

          if (saveParameters && saveParameters->ShouldSave()) {
            mutex.lock_shared();
            saveParameters->Save(graph);
            mutex.unlock_shared();
          }

          idleContext = &context;
          break;
        }
      }
    } while (idleContext == nullptr);

    // Put the idle thread to work
    const Game::Game state = *frontier.begin();
    frontier.erase(state);

    idleContext->thread = std::async(
        std::launch::async, [state, idleContext, &graph, depth, &mutex]() {
          Explore(std::move(state), idleContext->LocalVertices,
                  idleContext->Frontier, graph.Vertices, 0, depth, mutex);
        });
  } while (!frontier.empty() || anyThreadActive());

  Strategies::RetrogradeAnalyse(graph);
}

}  // namespace Strategies
}  // namespace StateGraph
