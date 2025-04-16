#include <fstream>
#include <memory>

#include "stateGraph.h"

namespace StateGraph {

template <typename T>
static void Write(std::ofstream& stream, const T value) {
  std::unique_ptr<T> pointer = std::make_unique<T>(value);
  stream.write((const char*)pointer.get(), sizeof(T));
}

template <>
void Write<std::optional<bool>>(std::ofstream& stream,
                                const std::optional<bool> val) {
  if (!val) {
    Write<uint8_t>(stream, 0xFF);
  } else {
    Write<uint8_t>(stream, val.value() ? 0x01 : 0x00);
  }
}

template <>
void Write<Game::GameSerialization>(
    std::ofstream& stream, const Game::GameSerialization serialization) {
  std::string bitString = serialization.to_string();
  constexpr std::size_t remainder = Game::GAME_SERIALIZATION_SIZE % 8;
  if (remainder != 0) bitString += std::string(8 - remainder, '0');

  for (size_t offset = 0; offset < Game::GAME_SERIALIZATION_SIZE; offset += 8) {
    const std::bitset<8> chunkBits(bitString, offset, 8);
    Write<uint8_t>(stream, (uint8_t)chunkBits.to_ulong());
  }
}

template <>
void Write<std::optional<WinState>>(std::ofstream& stream,
                                    const std::optional<WinState> winState) {
  if (!winState) {
    Write<int8_t>(stream, 0x00);
  } else {
    switch (winState.value()) {
      case WinState::Win:
        Write<int8_t>(stream, 0x01);
        break;
      case WinState::Draw:
        Write<int8_t>(stream, 0x02);
        break;
      case WinState::Lose:
        Write<int8_t>(stream, 0x03);
        break;
    }
  }
}

template <>
void Write<Game::Card>(std::ofstream& stream, const Game::Card card) {
  Write<size_t>(stream, (size_t)card.Type);
}

template <>
void Write<Game::Move>(std::ofstream& stream, const Game::Move move) {
  Write<size_t>(stream, move.PawnId);
  Write<Game::Card>(stream, move.UsedCard);
  Write<size_t>(stream, move.OffsetId);
}

template <>
void Write<Edge>(std::ofstream& stream, const Edge edge) {
  std::shared_ptr<Vertex> target = edge.Target.lock();
  Game::GameSerialization targetSerialization =
      target == nullptr ? Game::GameSerialization(0) : target->Serialization;
  Write<Game::GameSerialization>(stream, targetSerialization);

  Write<Game::Move>(stream, edge.Move);
  Write<std::optional<bool>>(stream, edge.Optimal);
}

template <>
void Write<Vertex>(std::ofstream& stream, const Vertex vertex) {
  Write<Game::GameSerialization>(stream, vertex.Serialization);
  Write<std::optional<WinState>>(stream, vertex.Quality);

  Write<size_t>(stream, vertex.Edges.size());
  for (const std::shared_ptr<Edge> edge : vertex.Edges) {
    Write<Edge>(stream, *edge);
  }
}

void Graph::SaveForwardRetrogradeAnalysis(
    const std::filesystem::path& path, ForwardRetrogradeProgress&& progress) {
  std::cout << "Saving current state graph..." << std::endl;

  std::ofstream stream;
  stream.open(path, std::ios::out | std::ios::binary);

  // Write call stack
  Write<size_t>(stream, progress.CallStack.size());
  for (Game::GameSerialization game : progress.CallStack) {
    Write<Game::GameSerialization>(stream, std::move(game));
  }

  // Write expanded vertices
  Write<size_t>(stream, progress.ExpandedVertices.size());
  for (const std::shared_ptr<Vertex> vertex : progress.ExpandedVertices) {
    Write<Game::GameSerialization>(stream, vertex->Serialization);
  }

  // Write unlabelled edges
  Write<size_t>(stream, progress.UnlabelledEdges.size());
  for (const std::shared_ptr<Edge> edge : progress.UnlabelledEdges) {
    const std::shared_ptr<Vertex> source = edge->Source.lock();
    const std::shared_ptr<Vertex> target = edge->Target.lock();
    if (source == nullptr || target == nullptr)
      throw std::runtime_error("Failed to lock edge while saving state!");

    Write<Game::GameSerialization>(stream, source->Serialization);
    Write<Game::GameSerialization>(stream, target->Serialization);
  }

  // Write graph
  Write<size_t>(stream, Vertices.size());
  for (const auto [_, vertex] : Vertices) {
    Write<Vertex>(stream, *vertex);
  }
}

struct EdgeInfo {
  std::optional<Game::GameSerialization> Source = std::nullopt;
  Game::GameSerialization Dest;
  Game::Move Move;
  std::optional<bool> Optimal;
};

struct VertexInfo {
  Game::GameSerialization Serialization;
  std::optional<WinState> Quality;
  std::vector<EdgeInfo> Edges;
};

template <typename T>
T Read(std::ifstream& stream) {
  T val;
  stream.read((char*)&val, sizeof(T));
  return val;
}

static bool GetBit(std::span<const uint8_t> bits, size_t pos) {
  const size_t byte = pos / 8;
  const uint8_t bitmask = 1 << (7 - pos % 8);

  return bits[byte] & bitmask;
}

template <>
std::optional<bool> Read(std::ifstream& stream) {
  const uint8_t byte = Read<uint8_t>(stream);
  switch (byte) {
    case 0xFF:
      return std::nullopt;
    case 0x00:
      return std::optional<bool>(false);
    case 0x01:
      return std::optional<bool>(true);

    default: {
      std::string err = std::format("Invalid optional bool \"{}\"!", byte);
      throw std::runtime_error(err);
    }
  }
}

template <>
Game::GameSerialization Read(std::ifstream& stream) {
  std::string bitString = "";

  constexpr size_t remainder = Game::GAME_SERIALIZATION_SIZE % 8;
  constexpr size_t byteCount =
      (Game::GAME_SERIALIZATION_SIZE / 8) + (remainder != 0);

  std::array<uint8_t, byteCount> data;
  stream.read((char*)data.data(), byteCount);

  for (size_t bit = 0; bit < Game::GAME_SERIALIZATION_SIZE; bit++) {
    bitString += GetBit(data, bit) ? '1' : '0';
  }

  return Game::GameSerialization(bitString);
}

template <>
std::optional<WinState> Read(std::ifstream& stream) {
  const uint8_t byte = Read<uint8_t>(stream);
  switch (byte) {
    case 0x00:
      return std::nullopt;
    case 0x01:
      return WinState::Win;
    case 0x02:
      return WinState::Draw;
    case 0x03:
      return WinState::Lose;

    default: {
      std::string err = std::format("Invalid win state \"{}\"!", byte);
      throw std::runtime_error(err);
    }
  }
}

template <>
Game::Card Read<Game::Card>(std::ifstream& stream) {
  const size_t cardType = Read<size_t>(stream);
  return Game::Card{(Game::CardType)cardType};
}

template <>
Game::Move Read<Game::Move>(std::ifstream& stream) {
  Game::Move move;
  move.PawnId = Read<size_t>(stream);
  move.UsedCard = Read<Game::Card>(stream);
  move.OffsetId = Read<size_t>(stream);

  return move;
}

template <>
EdgeInfo Read<EdgeInfo>(std::ifstream& stream) {
  EdgeInfo info;
  info.Dest = Read<Game::GameSerialization>(stream);
  info.Move = Read<Game::Move>(stream);
  info.Optimal = Read<std::optional<bool>>(stream);

  return info;
}

template <>
VertexInfo Read<VertexInfo>(std::ifstream& stream) {
  VertexInfo info;
  info.Serialization = Read<Game::GameSerialization>(stream);
  info.Quality = Read<std::optional<WinState>>(stream);

  const size_t edgeCount = Read<size_t>(stream);
  for (size_t edgeId = 0; edgeId < edgeCount; edgeId++) {
    info.Edges.emplace_back(Read<EdgeInfo>(stream));
  }

  return info;
}

std::pair<Graph, ForwardRetrogradeProgress>
Graph::LoadForwardRetrogradeAnalysis(const std::filesystem::path& path) {
  if (!std::filesystem::is_regular_file(path)) {
    const std::string err =
        std::format("\"{}\" is not a regular file!", path.string());
    throw std::runtime_error(err);
  }

  std::ifstream stream;
  stream.open(path, std::ios::in | std::ios::binary);

  ForwardRetrogradeProgress progress;
  Graph graph;

  // Parse callstack
  const size_t callStackDepth = Read<size_t>(stream);
  for (size_t depth = 0; depth < callStackDepth; depth++) {
    progress.CallStack.emplace_back(Read<Game::GameSerialization>(stream));
  }

  // Parse expanded vertices
  const size_t expandedVerticesCount = Read<size_t>(stream);
  std::vector<Game::GameSerialization> expandedVertices;
  expandedVertices.reserve(expandedVerticesCount);
  for (size_t vertexId = 0; vertexId < expandedVerticesCount; vertexId++) {
    expandedVertices.emplace_back(Read<Game::GameSerialization>(stream));
  }

  // Parse unlabelled edges
  const size_t unlabelledEdgesCount = Read<size_t>(stream);
  std::vector<std::pair<Game::GameSerialization, Game::GameSerialization>>
      unlabelledEdges;
  unlabelledEdges.reserve(unlabelledEdgesCount);
  for (size_t edgeId = 0; edgeId < unlabelledEdgesCount; edgeId++) {
    Game::GameSerialization&& source = Read<Game::GameSerialization>(stream);
    Game::GameSerialization&& target = Read<Game::GameSerialization>(stream);
    unlabelledEdges.emplace_back(std::move(source), std::move(target));
  }

  // Parse graph
  const size_t vertexCount = Read<size_t>(stream);
  std::vector<VertexInfo> vertices;
  vertices.reserve(vertexCount);
  for (size_t vertexId = 0; vertexId < vertexCount; vertexId++) {
    vertices.emplace_back(Read<VertexInfo>(stream));
  }

  // Construct graph
  for (const VertexInfo vertex : vertices) {
    Game::Game&& game = Game::Game::FromSerialization(vertex.Serialization);
    graph.Vertices.emplace(
        std::move(game),
        std::make_shared<Vertex>(vertex.Serialization, vertex.Quality));
  }

  for (const VertexInfo vertexInfo : vertices) {
    Game::Game game = Game::Game::FromSerialization(vertexInfo.Serialization);
    std::shared_ptr<Vertex> vertex = graph.Vertices.at(game);
    vertex->Edges.reserve(vertexInfo.Edges.size());

    for (const EdgeInfo edgeInfo : vertexInfo.Edges) {
      Game::Game&& destGame = Game::Game::FromSerialization(edgeInfo.Dest);
      std::shared_ptr<Vertex> destVertex = graph.Vertices.at(destGame);

      vertex->Edges.emplace_back(std::make_shared<Edge>(
          vertex, std::move(destVertex), edgeInfo.Move, edgeInfo.Optimal));
    }
  }

  // Map expanded vertices
  for (const Game::GameSerialization vertex : expandedVertices) {
    progress.ExpandedVertices.emplace(
        graph.Vertices.at(Game::Game::FromSerialization(vertex)));
  }

  // Map unlabelled edges
  for (const auto [sourceGame, targetGame] : unlabelledEdges) {
    const std::shared_ptr<Vertex> sourceVertex =
        graph.Vertices.at(Game::Game::FromSerialization(sourceGame));
    progress.UnlabelledEdges.emplace(sourceVertex->GetEdge(targetGame).value());
  }

  return {graph, progress};
}

}  // namespace StateGraph
