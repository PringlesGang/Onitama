#include "saveSystem.h"

#include <iostream>

#include "../../src/util/base64.h"
#include "../assertEqual.h"

namespace Tests {
namespace StateGraph {

using namespace ::StateGraph;

struct EdgeInfo {
  Game::GameSerialization Dest;
  Game::Move Move;
  std::optional<bool> Optimal;
};

struct VertexInfo {
  Game::GameSerialization Serialization;
  std::optional<WinState> Quality;
  std::vector<EdgeInfo> Edges;
};

int Save() { return Pass; }

int Load() {
  const Graph graph = Graph::Load("./tests/resources/load.graph");

  constexpr size_t expectedVertexCount = 9;

  if (graph.Vertices.size() != expectedVertexCount) {
    std::cerr << std::format("Expected {} vertices; got {}!",
                             expectedVertexCount, graph.Vertices.size())
              << std::endl;
    return Fail;
  }

  constexpr std::array<Game::GameSerialization, expectedVertexCount>
      expectedSerializations = {
          Game::GameSerialization(
              "100000101000000000001011010110110000100001000001"),
          Game::GameSerialization(
              "100000010000001000011011010110100100100100000000"),
          Game::GameSerialization(
              "100000101000000000011011010110110000100000000100"),
          Game::GameSerialization(
              "100000101000010000001011010110110000100000000100"),
          Game::GameSerialization(
              "100000010000000000011011010110100100100000010001"),
          Game::GameSerialization(
              "11000101000000110001011010001010000100000011011"),
          Game::GameSerialization(
              "100000010000000000101011010110100100100100000000"),
          Game::GameSerialization(
              "100110001000000000010011010110100100100100000000"),
          Game::GameSerialization(
              "100110001000010000011011010110100100100100000000"),
      };

  const std::array<VertexInfo, expectedVertexCount> expectedGraph = {
      VertexInfo{
          .Serialization = expectedSerializations[0],
          .Quality = std::nullopt,
          .Edges =
              {
                  EdgeInfo{
                      .Dest = expectedSerializations[2],
                      .Move =
                          Game::Move{0, Game::Card{Game::CardType::Crab}, 1},
                      .Optimal = std::optional<bool>(false),
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[3],
                      .Move =
                          Game::Move{1, Game::Card{Game::CardType::Crab}, 1},
                      .Optimal = std::nullopt,
                  },
              },
      },
      VertexInfo{
          .Serialization = expectedSerializations[1],
          .Quality = std::nullopt,
          .Edges = {},
      },
      VertexInfo{
          .Serialization = expectedSerializations[2],
          .Quality = WinState::Win,
          .Edges =
              {
                  EdgeInfo{
                      .Dest = expectedSerializations[4],
                      .Move =
                          Game::Move{0, Game::Card{Game::CardType::Goose}, 0},
                      .Optimal = std::optional<bool>(false),
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[5],
                      .Move =
                          Game::Move{1, Game::Card{Game::CardType::Rabbit}, 1},
                      .Optimal = std::optional<bool>(true),
                  },
              },
      },
      VertexInfo{
          .Serialization = expectedSerializations[3],
          .Quality = std::nullopt,
          .Edges = {},
      },
      VertexInfo{
          .Serialization = expectedSerializations[4],
          .Quality = WinState::Win,
          .Edges =
              {
                  EdgeInfo{
                      .Dest = expectedSerializations[6],
                      .Move =
                          Game::Move{0, Game::Card{Game::CardType::Boar}, 1},
                      .Optimal = std::optional<bool>(true),
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[7],
                      .Move =
                          Game::Move{0, Game::Card{Game::CardType::Boar}, 2},
                      .Optimal = std::optional<bool>(false),
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[1],
                      .Move = Game::Move{1, Game::Card{Game::CardType::Boar},
                                         0},
                      .Optimal = std::optional<bool>(false),
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[8],
                      .Move = Game::Move{1, Game::Card{Game::CardType::Boar},
                                         1},
                      .Optimal = std::optional<bool>(false),
                  },
              },
      },
      VertexInfo{
          .Serialization = expectedSerializations[5],
          .Quality = WinState::Lose,
          .Edges = {},
      },
      VertexInfo{
          .Serialization = expectedSerializations[6],
          .Quality = WinState::Lose,
          .Edges = {},
      },
      VertexInfo{
          .Serialization = expectedSerializations[7],
          .Quality = WinState::Lose,
          .Edges = {},
      },
      VertexInfo{
          .Serialization = expectedSerializations[8],
          .Quality = WinState::Lose,
          .Edges = {},
      },
  };

  for (const VertexInfo vertexInfo : expectedGraph) {
    const std::optional<std::weak_ptr<const Vertex>> weakVertex =
        graph.Get(Game::Game::FromSerialization(vertexInfo.Serialization));

    if (!weakVertex) {
      std::cerr << std::format("Missing vertex \"{}\"!",
                               Base64::Encode(vertexInfo.Serialization))
                << std::endl;
      return Fail;
    }

    const std::shared_ptr<const Vertex> vertex = weakVertex.value().lock();
    if (vertex == nullptr) {
      std::cerr << std::format("Failed to lock \"{}\"!",
                               Base64::Encode(vertexInfo.Serialization))
                << std::endl;
      return Fail;
    }

    if (vertex->Quality != vertexInfo.Quality) {
      const auto toString = [](const std::optional<WinState> quality) {
        if (!quality) return "Unknown";
        switch (quality.value()) {
          case WinState::Win:
            return "Win";
          case WinState::Draw:
            return "Draw";
          case WinState::Lose:
            return "Lose";
        }

        const std::string err =
            std::format("Invalid win state \"{}\"!", (int8_t)quality.value());
        throw std::runtime_error(err);
      };

      std::cerr << std::format("Expected quality {} for vertex \"{}\"; got {}!",
                               toString(vertexInfo.Quality),
                               Base64::Encode(vertexInfo.Serialization),
                               toString(vertex->Quality))
                << std::endl;
      return Fail;
    }

    if (vertex->Edges.size() != vertexInfo.Edges.size()) {
      std::cerr << std::format("Expected {} edges for vertex \"{}\"; got {}!",
                               vertexInfo.Edges.size(),
                               Base64::Encode(vertexInfo.Serialization),
                               vertex->Edges.size())
                << std::endl;
      return Fail;
    }

    for (const EdgeInfo edgeInfo : vertexInfo.Edges) {
      const Game::GameSerialization destGame = edgeInfo.Dest;
      const auto sameEdge = [destGame](std::shared_ptr<const Edge> edge) {
        const std::shared_ptr<const Vertex> target = edge->Target.lock();
        return target != nullptr && target->Serialization == destGame;
      };

      const auto edgeIt =
          std::find_if(vertex->Edges.begin(), vertex->Edges.end(), sameEdge);
      if (edgeIt == vertex->Edges.end()) {
        std::cerr << std::format("Failed to find edge from \"{}\" to \"{}\"!",
                                 Base64::Encode(vertexInfo.Serialization),
                                 Base64::Encode(destGame));
        return Fail;
      }
      const std::shared_ptr<const Edge> edge = *edgeIt;

      if (edge->Move != edgeInfo.Move) {
        const std::string expectedMove = std::format(
            "({}, {}, {})", edgeInfo.Move.PawnId,
            edgeInfo.Move.UsedCard.GetName(), edgeInfo.Move.OffsetId);
        const std::string receivedMove =
            std::format("({}, {}, {})", edge->Move.PawnId,
                        edge->Move.UsedCard.GetName(), edge->Move.OffsetId);

        std::cerr << std::format(
                         "Edge \"{}\" -> \"{}\" had move {} instead of {}!",
                         Base64::Encode(vertexInfo.Serialization),
                         Base64::Encode(destGame), receivedMove, expectedMove)
                  << std::endl;
        return Fail;
      }

      if (edge->Optimal != edgeInfo.Optimal) {
        const std::string expectedQuality =
            edgeInfo.Optimal
                ? (edgeInfo.Optimal.value() ? "Optimal" : "Redundant")
                : "Unknown";
        const std::string receivedQuality =
            edgeInfo.Optimal
                ? (edgeInfo.Optimal.value() ? "Optimal" : "Redundant")
                : "Unknown";

        std::cerr
            << std::format(
                   "Edge \"{}\" -> \"{}\" was marked \"{}\" instead of \"{}\"!",
                   Base64::Encode(vertexInfo.Serialization),
                   Base64::Encode(destGame), receivedQuality, expectedQuality)
            << std::endl;
        return Fail;
      }
    }
  }

  return Pass;
}

}  // namespace StateGraph
}  // namespace Tests
