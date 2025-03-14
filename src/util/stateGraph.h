#pragma once

#include <filesystem>

#include "../game/game.h"
#include "winState.h"

namespace StateGraph {

struct Vertex {
  Vertex(const Game::Game& game);
  Vertex(Game::GameSerialization serialization,
         std::optional<Game::Move> optimalMove = std::nullopt,
         WinState quality = WinState::Unknown);

  bool operator==(const Vertex& other) const {
    return Serialization == other.Serialization;
  }
  bool operator==(const Game::Game& game) const {
    return Serialization == game.Serialize();
  }

  const Game::GameSerialization Serialization;

  std::unordered_map<Game::Move, std::weak_ptr<const Vertex>> Edges;

  std::optional<Game::Move> OptimalMove = std::nullopt;
  WinState Quality = WinState::Unknown;
};

struct Edge {
  std::weak_ptr<Vertex> Source;
  std::weak_ptr<const Vertex> Target;

  Game::Move Move;
  bool Optimal = false;
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

  static Graph Import(const std::filesystem::path& nodesPath,
                      const std::filesystem::path& edgesPath);
  void Export(const std::filesystem::path& nodesPath,
              const std::filesystem::path& edgesPath) const;

 private:
  std::optional<Edge> ParseEdge(std::istringstream string) const;

  std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash, EqualTo>
      Vertices;
};

inline std::shared_ptr<Graph> SharedGameStateGraph = std::make_shared<Graph>();

}  // namespace StateGraph
