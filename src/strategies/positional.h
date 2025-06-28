#pragma once

#include <functional>
#include <unordered_map>

#include "../stateGraph/stateGraph.h"
#include "../util/parse.h"
#include "strategy.h"

namespace Strategy {

class Positional : public Strategy {
 public:
  Positional();
  Positional(std::shared_ptr<StateGraph::Graph> graph);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Positional>()>> Parse(
      std::istringstream& stream);

  constexpr static std::string GetName() { return "positional"; }

  constexpr static std::string GetHelpEntry() {
    constexpr std::string_view name =
        "Positional [{-i | --import} <nodes-path> <edges-path>]";
    constexpr std::array<std::string_view, 4> description{
        "Pick the optimal move as dictated by a state graph.",
        "If the optimal move is not yet defined, find it by",
        "applying forward-looking retrograde analysis.",
        "Optionally import a pre-computed state graph."};
    return Parse::PadCommandName(name, description);
  }

  std::shared_ptr<const StateGraph::Graph> GetGraph() const { return Graph; }
  void SetGraph(std::shared_ptr<StateGraph::Graph> graph) { Graph = graph; }

 private:
  std::shared_ptr<StateGraph::Graph> Graph;
};

}  // namespace Strategy
