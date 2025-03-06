#pragma once

#include <functional>
#include <unordered_map>

#include "../util/winState.h"
#include "strategy.h"

namespace Strategy {

class GameStateInfo {
 public:
  GameStateInfo(const Game::Game& game);

  bool operator==(const GameStateInfo& other) const {
    return Serialization == other.Serialization;
  }
  bool operator==(const Game::Game& game) const {
    return Serialization == game.Serialize();
  }

  Game::Move GetOptimalMove() const { return OptimalMove->first; }

  const Game::GameSerialization Serialization;

  std::optional<std::pair<Game::Move, std::weak_ptr<const GameStateInfo>>>
      OptimalMove = std::nullopt;
  WinState Quality = WinState::Unknown;
};

class GameStateGraph {
 public:
  std::optional<std::weak_ptr<const GameStateInfo>> Get(
      const Game::Game& game) const;
  std::weak_ptr<const GameStateInfo> Add(Game::Game&& game);

 private:
  std::unordered_map<Game::Game, std::shared_ptr<GameStateInfo>> Vertices;
};

inline std::shared_ptr<GameStateGraph> SharedGameStateGraph =
    std::make_shared<GameStateGraph>();

class Positional : public Strategy {
 public:
  Positional();
  Positional(std::shared_ptr<GameStateGraph> graph);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Positional>()>> Parse(
      std::istringstream& command);

  static std::string GetName();
  static std::string GetCommand();
  static std::string GetDescription();

  std::shared_ptr<const GameStateGraph> GetGraph() const { return Graph; }
  void SetGraph(std::shared_ptr<GameStateGraph> graph) { Graph = graph; }

 private:
  std::shared_ptr<GameStateGraph> Graph;
};

}  // namespace Strategy
