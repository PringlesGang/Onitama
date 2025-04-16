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

int SaveForwardRetrogradeAnalysis() {
  const std::filesystem::path outPath = "./Tests_StateGraph_Save_output";
  std::filesystem::remove(outPath);

  constexpr std::array<Game::GameSerialization, 9> serializations = {
      Game::GameSerialization(
          "100000101000000000001011010110110000100001000001"),  // goALWwhB
      Game::GameSerialization(
          "100000010000001000011011010110100100100100000000"),  // gQIbWkkA
      Game::GameSerialization(
          "100000101000000000011011010110110000100000000100"),  // goAbWwgE
      Game::GameSerialization(
          "100000101000010000001011010110110000100000000100"),  // goQLWwgE
      Game::GameSerialization(
          "100000010000000000011011010110100100100000010001"),  // gQAbWkgR
      Game::GameSerialization(
          "011000101000000110001011010001010000100000011011"),  // YoGLRQgb
      Game::GameSerialization(
          "100000010000000000101011010110100100100100000000"),  // gQArWkkA
      Game::GameSerialization(
          "100110001000000000010011010110100100100100000000"),  // mIATWkkA
      Game::GameSerialization(
          "100110001000010000011011010110100100100100000000"),  // mIQbWkkA
  };
  const std::array<std::shared_ptr<Vertex>, 9> vertices = {
      std::make_shared<Vertex>(serializations[0], std::nullopt),
      std::make_shared<Vertex>(serializations[1], std::nullopt),
      std::make_shared<Vertex>(serializations[2], WinState::Win),
      std::make_shared<Vertex>(serializations[3], std::nullopt),
      std::make_shared<Vertex>(serializations[4], WinState::Win),
      std::make_shared<Vertex>(serializations[5], WinState::Lose),
      std::make_shared<Vertex>(serializations[6], WinState::Lose),
      std::make_shared<Vertex>(serializations[7], WinState::Lose),
      std::make_shared<Vertex>(serializations[8], WinState::Lose),
  };

  vertices[0]->Edges = {
      std::make_shared<Edge>(vertices[0], vertices[2],
                             Game::Move{0, Game::Card{Game::CardType::Crab}, 1},
                             false),
      std::make_shared<Edge>(vertices[0], vertices[3],
                             Game::Move{1, Game::Card{Game::CardType::Crab}, 1},
                             std::nullopt),
  };
  vertices[2]->Edges = {
      std::make_shared<Edge>(
          vertices[2], vertices[4],
          Game::Move{0, Game::Card{Game::CardType::Goose}, 0}, false),
      std::make_shared<Edge>(
          vertices[2], vertices[5],
          Game::Move{1, Game::Card{Game::CardType::Rabbit}, 1}, true),
  };
  vertices[4]->Edges = {
      std::make_shared<Edge>(vertices[4], vertices[6],
                             Game::Move{0, Game::Card{Game::CardType::Boar}, 1},
                             false),
      std::make_shared<Edge>(vertices[4], vertices[7],
                             Game::Move{0, Game::Card{Game::CardType::Boar}, 2},
                             true),
      std::make_shared<Edge>(vertices[4], vertices[1],
                             Game::Move{1, Game::Card{Game::CardType::Boar}, 0},
                             false),
      std::make_shared<Edge>(vertices[4], vertices[8],
                             Game::Move{1, Game::Card{Game::CardType::Boar}, 1},
                             false),
  };

  Graph graph;
  graph.Vertices.reserve(vertices.size());
  for (const std::shared_ptr<Vertex> vertex : vertices) {
    graph.Vertices.emplace(Game::Game::FromSerialization(vertex->Serialization),
                           vertex);
  }

  const ForwardRetrogradeProgress progress{
      .ExpandedVertices = {vertices[0], vertices[4], vertices[2], vertices[6]},
      .UnlabelledEdges = {vertices[0]->Edges[1]},
      .CallStack = {serializations[0], serializations[2], serializations[4],
                    serializations[6]},
  };

  graph.SaveForwardRetrogradeAnalysis(outPath,
                                      ForwardRetrogradeProgress(progress));

  if (!std::filesystem::is_regular_file(outPath)) {
    std::cerr << "Could not find output file!" << std::endl;
    return Fail;
  }

  // Compare graphs
  const auto [loadedGraph, loadedProgress] =
      Graph::LoadForwardRetrogradeAnalysis(outPath);

  for (const auto [expectedGame, expectedVertex] : graph.Vertices) {
    const std::optional<std::weak_ptr<const Vertex>> weakVertex =
        loadedGraph.Get(expectedGame);

    if (!weakVertex) {
      std::cerr << std::format("Missing vertex \"{}\"!",
                               Base64::Encode(expectedVertex->Serialization))
                << std::endl;
      return Fail;
    }

    const std::shared_ptr<const Vertex> vertex = weakVertex.value().lock();
    if (vertex == nullptr) {
      std::cerr << std::format("Failed to lock \"{}\"!",
                               Base64::Encode(expectedVertex->Serialization))
                << std::endl;
      return Fail;
    }

    if (vertex->Quality != expectedVertex->Quality) {
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
                               toString(expectedVertex->Quality),
                               Base64::Encode(expectedVertex->Serialization),
                               toString(vertex->Quality))
                << std::endl;
      return Fail;
    }

    if (vertex->Edges.size() != expectedVertex->Edges.size()) {
      std::cerr << std::format("Expected {} edges for vertex \"{}\"; got {}!",
                               expectedVertex->Edges.size(),
                               Base64::Encode(expectedVertex->Serialization),
                               vertex->Edges.size())
                << std::endl;
      return Fail;
    }

    for (const std::shared_ptr<Edge> expectedEdge : expectedVertex->Edges) {
      const std::shared_ptr<const Vertex> expectedTarget =
          expectedEdge->Target.lock();
      if (expectedTarget == nullptr) {
        std::cerr << "Failed to lock target!" << std::endl;
        return Fail;
      }

      const Game::GameSerialization destGame = expectedTarget->Serialization;
      const auto sameEdge = [destGame](std::shared_ptr<const Edge> edge) {
        const std::shared_ptr<const Vertex> target = edge->Target.lock();
        return target != nullptr && target->Serialization == destGame;
      };

      const auto edgeIt =
          std::find_if(vertex->Edges.begin(), vertex->Edges.end(), sameEdge);
      if (edgeIt == vertex->Edges.end()) {
        std::cerr << std::format("Failed to find edge from \"{}\" to \"{}\"!",
                                 Base64::Encode(expectedVertex->Serialization),
                                 Base64::Encode(destGame));
        return Fail;
      }
      const std::shared_ptr<const Edge> edge = *edgeIt;

      if (edge->Move != expectedEdge->Move) {
        const std::string expectedMove = std::format(
            "({}, {}, {})", expectedEdge->Move.PawnId,
            expectedEdge->Move.UsedCard.GetName(), expectedEdge->Move.OffsetId);
        const std::string receivedMove =
            std::format("({}, {}, {})", edge->Move.PawnId,
                        edge->Move.UsedCard.GetName(), edge->Move.OffsetId);

        std::cerr << std::format(
                         "Edge \"{}\" -> \"{}\" had move {} instead of {}!",
                         Base64::Encode(expectedVertex->Serialization),
                         Base64::Encode(destGame), receivedMove, expectedMove)
                  << std::endl;
        return Fail;
      }

      if (edge->Optimal != expectedEdge->Optimal) {
        const std::string expectedQuality =
            expectedEdge->Optimal
                ? (expectedEdge->Optimal.value() ? "Optimal" : "Redundant")
                : "Unknown";
        const std::string receivedQuality =
            edge->Optimal ? (edge->Optimal.value() ? "Optimal" : "Redundant")
                          : "Unknown";

        std::cerr
            << std::format(
                   "Edge \"{}\" -> \"{}\" was marked \"{}\" instead of \"{}\"!",
                   Base64::Encode(expectedVertex->Serialization),
                   Base64::Encode(destGame), receivedQuality, expectedQuality)
            << std::endl;
        return Fail;
      }
    }
  }

  // Compare expanded vertices
  if (progress.ExpandedVertices.size() !=
      loadedProgress.ExpandedVertices.size()) {
    std::cerr << std::format("Expected {} expanded vertices; got {}!",
                             progress.ExpandedVertices.size(),
                             loadedProgress.ExpandedVertices.size())
              << std::endl;
    return Fail;
  }

  for (std::shared_ptr<Vertex> vertexPtr : loadedProgress.ExpandedVertices) {
    const Vertex vertex = *vertexPtr;
    const auto derefEqual = [vertex](std::shared_ptr<Vertex> pointer) {
      return *pointer == vertex;
    };

    if (std::none_of(progress.ExpandedVertices.begin(),
                     progress.ExpandedVertices.end(), derefEqual)) {
      std::cerr << std::format("Unexpected vertex \"{}\"!",
                               Base64::Encode(vertex.Serialization))
                << std::endl;
      return Fail;
    }
  }

  // Compare unlabelled edges
  if (progress.UnlabelledEdges.size() !=
      loadedProgress.UnlabelledEdges.size()) {
    std::cerr << std::format("Expected {} unlabelled edges; got {}!",
                             progress.UnlabelledEdges.size(),
                             loadedProgress.UnlabelledEdges.size())
              << std::endl;
    return Fail;
  }

  for (std::shared_ptr<Edge> edgePtr : loadedProgress.UnlabelledEdges) {
    const Edge edge = *edgePtr;
    const auto derefEqual = [edge](std::shared_ptr<Edge> pointer) {
      const std::shared_ptr<Vertex> source1 = pointer->Source.lock();
      const std::shared_ptr<Vertex> source2 = edge.Source.lock();
      const std::shared_ptr<Vertex> target1 = pointer->Target.lock();
      const std::shared_ptr<Vertex> target2 = edge.Target.lock();

      return source1 && source2 && target1 && target2 && *source1 == *source2 &&
             *target1 == *target2;
    };

    if (std::none_of(progress.UnlabelledEdges.begin(),
                     progress.UnlabelledEdges.end(), derefEqual)) {
      const std::shared_ptr<Vertex> source = edge.Source.lock();
      const std::shared_ptr<Vertex> target = edge.Target.lock();

      if (source == nullptr || target == nullptr) {
        std::cerr << std::format("Loaded invalid edge!") << std::endl;
      } else {
        std::cerr << std::format("Unexpected edge \"{} -> {}\"!",
                                 Base64::Encode(source->Serialization),
                                 Base64::Encode(target->Serialization))
                  << std::endl;
      }

      return Fail;
    }
  }

  // Compare call stacks
  if (progress.CallStack != loadedProgress.CallStack) {
    std::cerr << "Call stacks not equal!\nExpected: {";

    for (const Game::GameSerialization game : progress.CallStack)
      std::cerr << Base64::Encode(game) << ", ";
    std::cerr << "}\nGot: ";

    for (const Game::GameSerialization game : loadedProgress.CallStack)
      std::cerr << Base64::Encode(game) << ", ";
    std::cerr << "}" << std::endl;

    return Fail;
  }

  std::filesystem::remove(outPath);
  return Pass;
}

int LoadForwardRetrogradeAnalysis() {
  const auto [graph, progress] =
      Graph::LoadForwardRetrogradeAnalysis("./tests/resources/load.graph");

  constexpr size_t expectedVertexCount = 9;

  // Compare graphs
  if (graph.Vertices.size() != expectedVertexCount) {
    std::cerr << std::format("Expected {} vertices; got {}!",
                             expectedVertexCount, graph.Vertices.size())
              << std::endl;
    return Fail;
  }

  constexpr std::array<Game::GameSerialization, expectedVertexCount>
      expectedSerializations = {
          Game::GameSerialization(
              "100000101000000000001011010110110000100001000001"),  // goALWwhB
          Game::GameSerialization(
              "100000010000001000011011010110100100100100000000"),  // gQIbWkkA
          Game::GameSerialization(
              "100000101000000000011011010110110000100000000100"),  // goAbWwgE
          Game::GameSerialization(
              "100000101000010000001011010110110000100000000100"),  // goQLWwgE
          Game::GameSerialization(
              "100000010000000000011011010110100100100000010001"),  // gQAbWkgR
          Game::GameSerialization(
              "011000101000000110001011010001010000100000011011"),  // YoGLRQgb
          Game::GameSerialization(
              "100000010000000000101011010110100100100100000000"),  // gQArWkkA
          Game::GameSerialization(
              "100110001000000000010011010110100100100100000000"),  // mIATWkkA
          Game::GameSerialization(
              "100110001000010000011011010110100100100100000000"),  // mIQbWkkA
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
                      .Optimal = false,
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
                      .Optimal = false,
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[5],
                      .Move =
                          Game::Move{1, Game::Card{Game::CardType::Rabbit}, 1},
                      .Optimal = true,
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
                      .Optimal = false,
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[7],
                      .Move =
                          Game::Move{0, Game::Card{Game::CardType::Boar}, 2},
                      .Optimal = true,
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[1],
                      .Move = Game::Move{1, Game::Card{Game::CardType::Boar},
                                         0},
                      .Optimal = false,
                  },
                  EdgeInfo{
                      .Dest = expectedSerializations[8],
                      .Move = Game::Move{1, Game::Card{Game::CardType::Boar},
                                         1},
                      .Optimal = false,
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
            edge->Optimal ? (edge->Optimal.value() ? "Optimal" : "Redundant")
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

  // Compare expanded vertices
  const std::array<VertexInfo, 4> expectedExpandedVertices = {
      expectedGraph[0], expectedGraph[4], expectedGraph[2], expectedGraph[6]};

  if (progress.ExpandedVertices.size() != expectedExpandedVertices.size()) {
    std::cerr << std::format("Expected {} expanded vertices; got {}!",
                             expectedExpandedVertices.size(),
                             progress.ExpandedVertices.size())
              << std::endl;
    return Fail;
  }

  for (const VertexInfo info : expectedExpandedVertices) {
    const auto derefEqual = [info](std::shared_ptr<Vertex> pointer) {
      return pointer->Serialization == info.Serialization;
    };

    if (std::none_of(progress.ExpandedVertices.begin(),
                     progress.ExpandedVertices.end(), derefEqual)) {
      std::cerr << std::format("Missing vertex \"{}\"!",
                               Base64::Encode(info.Serialization))
                << std::endl;
      return Fail;
    }
  }

  // Compare unlabelled edges
  const std::array<std::pair<Game::GameSerialization, Game::GameSerialization>,
                   1>
      expectedUnlabelledEdges = {
          std::pair(expectedSerializations[0], expectedGraph[0].Edges[1].Dest)};

  if (progress.UnlabelledEdges.size() != expectedUnlabelledEdges.size()) {
    std::cerr << std::format("Expected {} unlabelled edges; got {}!",
                             expectedUnlabelledEdges.size(),
                             progress.UnlabelledEdges.size())
              << std::endl;
    return Fail;
  }

  for (const auto [source, target] : expectedUnlabelledEdges) {
    const auto derefEqual = [source, target](std::shared_ptr<Edge> pointer) {
      const std::shared_ptr<Vertex> source2 = pointer->Source.lock();
      const std::shared_ptr<Vertex> target2 = pointer->Target.lock();

      return source2 && target2 && source2->Serialization == source &&
             target2->Serialization == target;
    };

    if (std::none_of(progress.UnlabelledEdges.begin(),
                     progress.UnlabelledEdges.end(), derefEqual)) {
      std::cerr << std::format("Missing edge \"{} -> {}\"!",
                               Base64::Encode(source), Base64::Encode(target))
                << std::endl;

      return Fail;
    }
  }

  // Compare call stacks
  const std::deque<Game::GameSerialization> expectedCallStack = {
      expectedSerializations[0], expectedSerializations[2],
      expectedSerializations[4], expectedSerializations[6]};

  if (progress.CallStack != expectedCallStack) {
    std::cerr << "Call stacks not equal!\nExpected: {";

    for (const Game::GameSerialization game : expectedCallStack)
      std::cerr << Base64::Encode(game) << ", ";
    std::cerr << "}\nGot: ";

    for (const Game::GameSerialization game : progress.CallStack)
      std::cerr << Base64::Encode(game) << ", ";
    std::cerr << "}" << std::endl;

    return Fail;
  }

  return Pass;
}

}  // namespace StateGraph
}  // namespace Tests
