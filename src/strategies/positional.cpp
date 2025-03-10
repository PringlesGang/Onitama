#include "positional.h"

#include <format>

#include "../cli/command.h"
#include "../util/parse.h"

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
    std::istringstream& stream) {
  std::string argument;
  stream >> argument;
  Parse::ToLower(argument);

  std::optional<std::shared_ptr<StateGraph::Graph>> graph = std::nullopt;

  if (!argument.empty()) {
    if (argument == "--import" || argument == "-i") {
      const std::optional<std::filesystem::path> path =
          Parse::ParsePath(stream);
      if (!path) return std::nullopt;

      graph = std::make_shared<StateGraph::Graph>(
          StateGraph::Graph::Import(path.value()));
    } else {
      Parse::Unparse(stream, argument);
    }
  }

  return [graph] {
    return graph ? std::make_unique<Positional>(graph.value())
                 : std::make_unique<Positional>();
  };
}

std::string Positional::GetName() { return "positional"; }

std::string Positional::GetCommand() {
  return std::format("{} (--import filepath)", GetName());
}

std::string Positional::GetDescription() {
  return "Slowly seeks out a perfect positional strategy to perform.\n"
         "A file storing a state graph may be provided.";
}

}  // namespace Strategy
