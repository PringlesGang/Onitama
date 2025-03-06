#pragma once

#include "../game/game.h"
#include "winState.h"

namespace StateGraph {

class Vertex {
 public:
  Vertex(const Game::Game& game);

  bool operator==(const Vertex& other) const {
    return Serialization == other.Serialization;
  }
  bool operator==(const Game::Game& game) const {
    return Serialization == game.Serialize();
  }

  Game::Move GetOptimalMove() const { return OptimalMove->first; }

  const Game::GameSerialization Serialization;

  std::optional<std::pair<Game::Move, std::weak_ptr<const Vertex>>>
      OptimalMove = std::nullopt;
  WinState Quality = WinState::Unknown;
};

struct Hash {
  size_t operator()(const Game::Game& game) const noexcept;
};

struct EqualTo {
  bool operator()(const Game::Game& first,
                  const Game::Game& second) const noexcept;
};

class Graph {
 public:
  std::optional<std::weak_ptr<const Vertex>> Get(const Game::Game& game) const;
  std::weak_ptr<const Vertex> Add(Game::Game&& game);

 private:
  std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash, EqualTo>
      Vertices;
};

inline std::shared_ptr<Graph> SharedGameStateGraph = std::make_shared<Graph>();

}  // namespace StateGraph
