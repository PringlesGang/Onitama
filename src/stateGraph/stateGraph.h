#pragma once

#include <chrono>
#include <deque>
#include <filesystem>
#include <iostream>
#include <unordered_set>

#include "../game/game.h"
#include "../util/winState.h"

namespace StateGraph {

struct Edge;

struct Vertex {
  Vertex(const Game::Game& game);
  Vertex(Game::GameSerialization serialization,
         std::optional<WinState> quality = std::nullopt);

  bool operator==(const Vertex& other) const {
    return Serialization == other.Serialization;
  }
  bool operator==(const Game::Game& game) const {
    return Serialization == game.Serialize();
  }

  void SetOptimalMove(Game::Move move);
  std::optional<Game::Move> GetOptimalMove() const;

  std::optional<std::shared_ptr<Edge>> GetEdge(Game::Move move);
  std::optional<std::shared_ptr<Edge>> GetEdge(Game::GameSerialization game);

  const Game::GameSerialization Serialization;

  std::vector<std::shared_ptr<Edge>> Edges;

  std::optional<WinState> Quality;
};

struct Edge {
  Edge(std::weak_ptr<Vertex> source, std::weak_ptr<Vertex> target,
       Game::Move move, std::optional<bool> optimal = std::nullopt);

  std::weak_ptr<Vertex> Source;
  std::weak_ptr<Vertex> Target;

  Game::Move Move;

  std::optional<bool> Optimal;

  bool IsOptimal() const { return Optimal.has_value() && Optimal.value(); }
};

struct Hash {
  size_t operator()(const Game::Game& game) const noexcept;
};

struct EqualTo {
  bool operator()(const Game::Game& first,
                  const Game::Game& second) const noexcept;
};

struct Graph {
 public:
  std::optional<std::weak_ptr<const Vertex>> Get(const Game::Game& game) const;

  void Save(const std::filesystem::path& path, size_t runtime = 0) const;
  static std::pair<Graph, std::chrono::duration<size_t>> Load(
      const std::filesystem::path& path);

  static Graph Import(const std::filesystem::path& nodesPath,
                      const std::filesystem::path& edgesPath);
  void Export(const std::filesystem::path& nodesPath,
              const std::filesystem::path& edgesPath) const;
  void ExportImages(const std::filesystem::path& imagesPath) const;

  std::unordered_map<Game::Game, std::shared_ptr<Vertex>, Hash, EqualTo>
      Vertices;

 private:
  std::optional<Edge> ParseEdge(std::istringstream string) const;
};

inline std::shared_ptr<Graph> SharedGameStateGraph = std::make_shared<Graph>();

}  // namespace StateGraph
