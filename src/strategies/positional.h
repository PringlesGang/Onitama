#pragma once

#include <functional>
#include <unordered_map>

#include "../stateGraph/stateGraph.h"
#include "strategy.h"

namespace Strategy {

class Positional : public Strategy {
 public:
  Positional();
  Positional(std::shared_ptr<StateGraph::Graph> graph);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Positional>()>> Parse(
      std::istringstream& stream);

  static std::string GetName();
  static std::string GetCommand();
  static std::string GetDescription();

  std::shared_ptr<const StateGraph::Graph> GetGraph() const { return Graph; }
  void SetGraph(std::shared_ptr<StateGraph::Graph> graph) { Graph = graph; }

 private:
  std::shared_ptr<StateGraph::Graph> Graph;
};

}  // namespace Strategy
