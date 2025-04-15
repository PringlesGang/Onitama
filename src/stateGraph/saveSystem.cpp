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
static void Write<std::optional<bool>>(std::ofstream& stream,
                                       const std::optional<bool> val) {
  if (!val) {
    Write<uint8_t>(stream, 0xFF);
  } else {
    Write<uint8_t>(stream, val.value() ? 0x01 : 0x00);
  }
}

template <size_t N>
static void WriteBits(std::ofstream& stream, const std::bitset<N> bits) {
  const std::lldiv_t byteDiv = std::lldiv(N, 8);
  const size_t byteCount = byteDiv.quot + (byteDiv.rem != 0);

  Write<size_t>(stream, byteCount);

  std::string bitString = bits.to_string();
  if (byteDiv.rem != 0) bitString += std::string(8 - byteDiv.rem, '0');

  for (size_t offset = 0; offset < N; offset += 8) {
    const std::bitset<8> chunkBits(bitString, offset, 8);
    Write<uint8_t>(stream, (uint8_t)chunkBits.to_ulong());
  }
}

template <>
static void Write<std::optional<WinState>>(
    std::ofstream& stream, const std::optional<WinState> winState) {
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
static void Write<Game::Card>(std::ofstream& stream, const Game::Card card) {
  Write<size_t>(stream, (size_t)card.Type);
}

template <>
static void Write<Game::Move>(std::ofstream& stream, const Game::Move move) {
  Write<size_t>(stream, move.PawnId);
  Write<Game::Card>(stream, move.UsedCard);
  Write<size_t>(stream, move.OffsetId);
}

template <>
static void Write<Edge>(std::ofstream& stream, const Edge edge) {
  std::shared_ptr<Vertex> target = edge.Target.lock();
  Game::GameSerialization targetSerialization =
      target == nullptr ? Game::GameSerialization(0) : target->Serialization;
  WriteBits<Game::GAME_SERIALIZATION_SIZE>(stream, targetSerialization);

  Write<Game::Move>(stream, edge.Move);
  Write<std::optional<bool>>(stream, edge.Optimal);
}

template <>
static void Write<Vertex>(std::ofstream& stream, const Vertex vertex) {
  WriteBits<Game::GAME_SERIALIZATION_SIZE>(stream, vertex.Serialization);
  Write<std::optional<WinState>>(stream, vertex.Quality);

  Write<size_t>(stream, vertex.Edges.size());
  for (const std::shared_ptr<Edge> edge : vertex.Edges) {
    Write<Edge>(stream, *edge);
  }
}

void Graph::Save(const std::filesystem::path& path) const {
  std::cout << "Saving current state graph..." << std::endl;

  std::ofstream stream;
  stream.open(path, std::ios::out | std::ios::binary);

  Write<size_t>(stream, Vertices.size());
  for (const auto [_, vertex] : Vertices) {
    Write<Vertex>(stream, *vertex);
  }
}

}  // namespace StateGraph
