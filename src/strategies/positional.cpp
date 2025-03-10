#include "positional.h"

namespace Strategy {

Positional::Positional() : Graph(StateGraph::SharedGameStateGraph) {}

Positional::Positional(std::shared_ptr<StateGraph::Graph> graph)
    : Graph(graph) {}

Game::Move Positional::GetMove(const Game::Game& game) {
  const std::optional<std::weak_ptr<const StateGraph::Vertex>> found =
      Graph->Get(game);

  // Either get the pre-computed positional strategy,
  // or compute the positional strategy now
  return found ? found->lock()->OptimalMove.value()
               : Graph->Add(Game::Game(game)).lock()->OptimalMove.value();
}

std::optional<std::function<std::unique_ptr<Positional>()>> Positional::Parse(
    std::istringstream& command) {
  return std::make_unique<Positional>;
}

std::string Positional::GetName() { return "positional"; }

std::string Positional::GetCommand() { return GetName(); }

std::string Positional::GetDescription() {
  return "Slowly seeks out a perfect positional strategy to perform.";
}

}  // namespace Strategy
