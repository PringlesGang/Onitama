#include "stateGraph.h"

#include <format>
#include <fstream>
#include <iostream>
#include <unordered_set>

#include "base64.h"

namespace StateGraph {

Vertex::Vertex(const Game::Game& game)
    : Serialization(game.Serialize()),
      Quality(game.IsFinished() ? WinState::Lost : WinState::Unknown) {}

Vertex::Vertex(Game::GameSerialization serialization,
               std::optional<Game::Move> optimalMove, WinState quality)
    : Serialization(serialization),
      OptimalMove(optimalMove),
      Quality(quality) {}

static bool CompareCoordinates(const Game::Game& first,
                               const Game::Game& second,
                               const Color firstPlayer,
                               const Color secondPlayer) {
  const bool masterCaptured = first.MasterCaptured(firstPlayer);
  if (masterCaptured != second.MasterCaptured(secondPlayer)) return false;

  const bool orient = firstPlayer != secondPlayer;
  const auto [width, height] = second.GetDimensions();
  const auto orientCoord = [orient, width, height](Coordinate coordinate) {
    return orient
               ? Coordinate(width - coordinate.x - 1, height - coordinate.y - 1)
               : coordinate;
  };

  const std::vector<Coordinate>& firstCoordinates =
      first.GetPawnCoordinates(firstPlayer);
  const std::vector<Coordinate>& secondCoordinates =
      second.GetPawnCoordinates(secondPlayer);

  if (firstCoordinates.size() != secondCoordinates.size()) return false;
  if (firstCoordinates.size() == 0) return true;

  if (!masterCaptured &&
      firstCoordinates[0] != orientCoord(secondCoordinates[0]))
    return false;

  auto secondCoord = orient ? --secondCoordinates.end()
                            : secondCoordinates.begin() + !masterCaptured;
  for (auto firstCoord = firstCoordinates.begin() + !masterCaptured;
       firstCoord != firstCoordinates.end(); firstCoord++) {
    if (*firstCoord != orientCoord(*secondCoord)) return false;

    if (!orient || secondCoord != secondCoordinates.begin())
      secondCoord += orient ? -1 : 1;
  }

  return true;
}

bool EqualTo::operator()(const Game::Game& first,
                         const Game::Game& second) const noexcept {
  if (first.GetSetAsideCard() != second.GetSetAsideCard()) return false;

  if (first.GetDimensions() != second.GetDimensions()) return false;

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color firstPlayer =
        playerId == 0 ? first.GetCurrentPlayer() : ~first.GetCurrentPlayer();
    const Color secondPlayer =
        playerId == 0 ? second.GetCurrentPlayer() : ~second.GetCurrentPlayer();

    const std::span<const Game::Card, HAND_SIZE> firstHand =
        first.GetHand(firstPlayer);
    const std::span<const Game::Card, HAND_SIZE> secondHand =
        first.GetHand(firstPlayer);
    if (std::unordered_multiset<Game::Card>(firstHand.begin(),
                                            firstHand.end()) !=
        std::unordered_multiset<Game::Card>(secondHand.begin(),
                                            secondHand.end()))
      return false;

    if (!CompareCoordinates(first, second, firstPlayer, secondPlayer))
      return false;
  }

  return true;
}

size_t Hash::operator()(const Game::Game& game) const noexcept {
  // Cards
  size_t hash = (size_t)game.GetSetAsideCard().Type;

  for (const Game::Card card : game.GetHand(TopPlayer)) {
    hash ^= (size_t)card.Type;
  }

  // Pawn locations
  const auto [width, height] = game.GetDimensions();
  const size_t boardEnd = width * height - 1;

  const bool orient = TopPlayer == game.GetCurrentPlayer();

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : ~TopPlayer;
    for (const Coordinate coordinate : game.GetPawnCoordinates(player)) {
      const size_t offset = coordinate.x + width * coordinate.y;
      hash ^= orient ? offset : boardEnd - offset;
    }
  }

  return hash;
}

std::optional<std::weak_ptr<const Vertex>> Graph::Get(
    const Game::Game& game) const {
  const Game::GameSerialization serialization = game.Serialize();
  return Vertices.contains(game)
             ? std::optional(std::weak_ptr(Vertices.at(game)))
             : std::nullopt;
};

std::weak_ptr<const Vertex> Graph::Add(Game::Game&& game) {
  const std::optional<std::weak_ptr<const Vertex>> storedInfo = Get(game);
  if (storedInfo) return storedInfo.value();

  std::shared_ptr<Vertex> info = std::make_shared<Vertex>(game);
  Vertices.emplace(game, info);

  // Terminal game state
  if (info->Quality != WinState::Unknown) return info;

  const std::vector<Game::Move>& validMoves = game.GetValidMoves();
  for (const Game::Move move : validMoves) {
    Game::Game nextState(game);
    nextState.DoMove(move);

    // Traverse further
    std::shared_ptr<const Vertex> nextInfo = Add(std::move(nextState)).lock();
    const WinState nextQuality = -nextInfo->Quality;

    // Select the best move yet
    if (!info->OptimalMove || info->Quality < nextQuality) {
      info->Quality = nextQuality;
      info->OptimalMove = move;
    }

    // A winning positional strategy has been found
    if (info->Quality == WinState::Won) return info;
  }

  return info;
}

static std::optional<Game::Move> ParseMove(std::istringstream& string) {
  // Pawn id
  std::string pawnIdString;
  std::getline(string, pawnIdString, ',');

  std::optional<size_t> pawnId;
  if (!pawnIdString.empty()) {
    try {
      pawnId = std::stoull(pawnIdString);
    } catch (std::invalid_argument e) {
      std::cerr << std::format("Failed to parse pawn id \"{}\"", pawnIdString)
                << std::endl;
      return std::nullopt;
    }
  }

  // Card
  std::string cardString;
  std::getline(string, cardString, ',');

  std::optional<Game::Card> card;
  if (!cardString.empty()) {
    try {
      size_t cardType = std::stoull(cardString);
      card = Game::Card{.Type = (Game::CardType)cardType};
    } catch (std::invalid_argument e) {
      std::cerr << std::format("Failed to parse card type \"{}\"", cardString)
                << std::endl;
      return std::nullopt;
    }
  }

  // Offset id
  std::string offsetIdString;
  std::getline(string, offsetIdString, ',');

  std::optional<size_t> offsetId;
  if (!offsetIdString.empty()) {
    try {
      offsetId = std::stoull(offsetIdString);
    } catch (std::invalid_argument e) {
      std::cerr << std::format("Failed to parse offset id \"{}\"",
                               offsetIdString)
                << std::endl;
      return std::nullopt;
    }
  }

  if (!pawnId && !card && !offsetId) return std::nullopt;

  if (!pawnId) {
    std::cerr << "Missing pawn id!" << std::endl;
    return std::nullopt;
  }
  if (!card) {
    std::cerr << "Missing card!" << std::endl;
    return std::nullopt;
  }
  if (!offsetId) {
    std::cerr << "Missing offset id!" << std::endl;
    return std::nullopt;
  }

  return Game::Move{.PawnId = pawnId.value(),
                    .UsedCard = card.value(),
                    .OffsetId = offsetId.value()};
}

static std::optional<Vertex> ParseVertex(std::istringstream string) {
  // Serialization
  std::string serializationString;
  std::getline(string, serializationString, ',');

  std::istringstream serializationStream(serializationString);
  const std::optional<Game::GameSerialization> parsedSerialization =
      Game::Game::ParseSerialization(serializationStream);
  if (!parsedSerialization) {
    std::cerr << std::format("Failed to parse serialization \"{}\"!",
                             serializationString)
              << std::endl;
    return std::nullopt;
  }

  const Game::GameSerialization serialization = parsedSerialization.value();

  // Optimal move
  const std::optional<Game::Move> optimalMove = ParseMove(string);

  // Quality
  std::string qualityValString;
  std::getline(string, qualityValString);

  WinState quality;
  try {
    const int8_t qualityVal = std::stoi(qualityValString);
    quality = (WinState)qualityVal;
  } catch (std::invalid_argument e) {
    std::cerr << std::format("Failed to parse quality \"{}\"", qualityValString)
              << std::endl;
    return std::nullopt;
  }

  return Vertex(std::move(serialization), std::move(optimalMove),
                std::move(quality));
}

Graph Graph::Import(const std::filesystem::path& filePath) {
  std::ifstream stream;
  stream.open(filePath);

  Graph graph;

  for (std::string vertexString; !stream.eof();
       std::getline(stream, vertexString)) {
    if (vertexString.empty()) continue;

    const std::optional<Vertex> vertex =
        ParseVertex(std::istringstream(vertexString));
    if (vertex) {
      graph.Vertices.insert(
          {Game::Game::FromSerialization(vertex->Serialization),
           std::make_shared<Vertex>(vertex.value())});
    }
  }

  return graph;
}

void Graph::Export(const std::filesystem::path& filePath) const {
  std::ofstream stream;
  stream.open(filePath);

  for (auto vertexIt = Vertices.begin(); vertexIt != Vertices.end();
       vertexIt++) {
    const Vertex& vertex = *vertexIt->second;

    const std::string serialization = Base64::Encode(vertex.Serialization);

    std::string optimalMove = ",,";
    if (vertex.OptimalMove) {
      const Game::Move move = vertex.OptimalMove.value();
      optimalMove = std::format("{},{},{}", move.PawnId,
                                (size_t)move.UsedCard.Type, move.OffsetId);
    }

    const std::string quality = std::to_string((int8_t)vertex.Quality);

    stream << std::format("{},{},{}", serialization, optimalMove, quality)
           << std::endl;
  }
}

}  // namespace StateGraph
