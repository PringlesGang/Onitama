#include "positional.h"

#include <cassert>
#include <format>

#include "../cli/command.h"
#include "../stateGraph/strategies.h"

namespace Strategy {

Positional::Positional() : Graph(StateGraph::SharedGameStateGraph) {}

Positional::Positional(std::shared_ptr<StateGraph::Graph> graph)
    : Graph(graph) {}

Game::Move Positional::GetMove(const Game::Game& game) {
  // Try to get a precomputed optimal move
  std::optional<std::weak_ptr<const StateGraph::Vertex>> found =
      Graph->Get(game);

  if (found.has_value()) {
    const std::shared_ptr<const StateGraph::Vertex> vertex = found->lock();
    assert(vertex != nullptr);

    const std::optional<Game::Move> optimalMove = vertex->GetOptimalMove();
    if (optimalMove.has_value()) return optimalMove.value();
  }

  // Compute the optimal move
  StateGraph::Strategies::ForwardRetrogradeAnalysis(*Graph, game);

  found = Graph->Get(game);

  if (found.has_value()) {
    const std::shared_ptr<const StateGraph::Vertex> vertex = found->lock();
    assert(vertex != nullptr);

    const std::optional<Game::Move> optimalMove = vertex->GetOptimalMove();
    if (optimalMove.has_value()) return optimalMove.value();
  }

  std::cerr << "Failed to find optimal move! Defaulting to first valid move."
            << std::endl;
  return game.GetValidMoves()[0];
}

std::optional<std::function<std::unique_ptr<Positional>()>> Positional::Parse(
    std::istringstream& stream) {
  std::string argument;
  stream >> argument;
  Parse::ToLower(argument);

  std::optional<std::shared_ptr<StateGraph::Graph>> graph = std::nullopt;

  if (!argument.empty()) {
    if (argument == "--import" || argument == "-i") {
      const std::optional<std::filesystem::path> nodesPath =
          Parse::ParsePath(stream);
      if (!nodesPath) return std::nullopt;

      const std::optional<std::filesystem::path> edgesPath =
          Parse::ParsePath(stream);
      if (!edgesPath) return std::nullopt;

      graph = std::make_shared<StateGraph::Graph>(
          StateGraph::Graph::Import(nodesPath.value(), edgesPath.value()));
    } else {
      Parse::Unparse(stream, argument);
    }
  }

  return [graph] {
    return graph ? std::make_unique<Positional>(graph.value())
                 : std::make_unique<Positional>();
  };
}

}  // namespace Strategy
