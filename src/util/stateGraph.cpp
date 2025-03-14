#include "stateGraph.h"

#include <format>
#include <fstream>
#include <iostream>

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

void Graph::ExploreComponentRecursive(
    std::weak_ptr<Vertex> weakVertex,
    std::unordered_set<Game::Game>& exploring) {
  std::shared_ptr<Vertex> vertex = weakVertex.lock();
  if (vertex == nullptr) return;

  Game::Game game = Game::Game::FromSerialization(vertex->Serialization);

  if (exploring.contains(game)) return;
  exploring.insert(game);

  for (Game::Move move : game.GetValidMoves()) {
    Game::Game nextState = game;
    nextState.DoMove(move);

    if (!Vertices.contains(nextState))
      Vertices.insert({nextState, std::make_shared<Vertex>(nextState)});
    std::weak_ptr<Vertex> nextVertex = Vertices.at(nextState);

    vertex->Edges.insert({move, nextVertex});
    ExploreComponentRecursive(nextVertex, exploring);
  }
}

std::weak_ptr<const Vertex> Graph::ExploreComponent(Game::Game&& game) {
  const std::shared_ptr<Vertex> vertex =
      Vertices.contains(game) ? Vertices.at(game)
                              : std::make_shared<Vertex>(std::move(game));

  std::unordered_set<Game::Game> exploring;
  ExploreComponentRecursive(vertex, exploring);

  return vertex;
}

std::weak_ptr<const Vertex> Graph::FindPerfectStrategy(Game::Game&& game) {
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
    std::shared_ptr<const Vertex> nextInfo =
        FindPerfectStrategy(std::move(nextState)).lock();
    info->Edges.emplace(move, nextInfo);

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

  return Vertex(std::move(serialization), std::nullopt, std::move(quality));
}

std::optional<Edge> Graph::ParseEdge(std::istringstream string) const {
  // Source serialization
  std::string sourceSerializationString;
  std::getline(string, sourceSerializationString, ',');

  std::istringstream sourceSerializationStream(sourceSerializationString);
  const std::optional<Game::GameSerialization> parsedSourceSerialization =
      Game::Game::ParseSerialization(sourceSerializationStream);
  if (!parsedSourceSerialization) {
    std::cerr << std::format("Failed to parse source serialization \"{}\"!",
                             sourceSerializationString)
              << std::endl;
    return std::nullopt;
  }
  const Game::Game sourceGame =
      Game::Game::FromSerialization(parsedSourceSerialization.value());

  if (!Vertices.contains(sourceGame)) {
    std::cerr << std::format("Undefined vertex \"{}\"!",
                             sourceSerializationString)
              << std::endl;
    return std::nullopt;
  }
  const std::weak_ptr<Vertex> source = Vertices.at(sourceGame);

  // Target serialization
  std::string targetSerializationString;
  std::getline(string, targetSerializationString, ',');

  std::istringstream targetSerializationStream(targetSerializationString);
  const std::optional<Game::GameSerialization> parsedTargetSerialization =
      Game::Game::ParseSerialization(targetSerializationStream);
  if (!parsedTargetSerialization) {
    std::cerr << std::format("Failed to parse target serialization \"{}\"!",
                             targetSerializationString)
              << std::endl;
    return std::nullopt;
  }
  const Game::Game targetGame =
      Game::Game::FromSerialization(parsedTargetSerialization.value());

  if (!Vertices.contains(targetGame)) {
    std::cerr << std::format("Undefined vertex \"{}\"!",
                             sourceSerializationString)
              << std::endl;
    return std::nullopt;
  }
  const std::weak_ptr<const Vertex> target = Vertices.at(targetGame);

  // Move
  const std::optional<Game::Move> move = ParseMove(string);
  if (!move) return std::nullopt;

  // Optimal move
  std::string optimalString;
  std::getline(string, optimalString);
  if (optimalString != "false" && optimalString != "true") {
    std::cerr << std::format("Failed to parse boolean \"{}\"!", optimalString)
              << std::endl;
    return std::nullopt;
  }
  const bool optimal = optimalString == "true";

  return Edge{.Source = source,
              .Target = target,
              .Move = move.value(),
              .Optimal = optimal};
}

Graph Graph::Import(const std::filesystem::path& nodesPath,
                    const std::filesystem::path& edgesPath) {
  Graph graph;

  std::ifstream nodesStream;
  nodesStream.open(nodesPath);

  for (std::string nodeString; !nodesStream.eof();
       std::getline(nodesStream, nodeString)) {
    if (nodeString.empty()) continue;

    const std::optional<Vertex> vertex =
        ParseVertex(std::istringstream(nodeString));
    if (vertex) {
      graph.Vertices.insert(
          {Game::Game::FromSerialization(vertex->Serialization),
           std::make_shared<Vertex>(vertex.value())});
    }
  }

  nodesStream.close();

  std::ifstream edgesStream;
  edgesStream.open(edgesPath);

  for (std::string edgeString; !edgesStream.eof();
       std::getline(edgesStream, edgeString)) {
    if (edgeString.empty()) continue;

    const std::optional<Edge> edge =
        graph.ParseEdge(std::istringstream(edgeString));
    if (!edge) continue;

    const std::shared_ptr<Vertex> source = edge->Source.lock();
    source->Edges.insert({edge->Move, edge->Target});

    if (edge->Optimal) source->OptimalMove = edge->Move;
  }

  edgesStream.close();

  return graph;
}

void Graph::Export(const std::filesystem::path& nodesPath,
                   const std::filesystem::path& edgesPath) const {
  std::ofstream nodesStream;
  nodesStream.open(nodesPath);

  std::ofstream edgesStream;
  edgesStream.open(edgesPath);

  // Headers
  nodesStream << "Id, Quality" << std::endl;
  edgesStream << "Source, Target, Pawn, Card, Offset, Optimal" << std::endl;

  for (auto vertexIt = Vertices.begin(); vertexIt != Vertices.end();
       vertexIt++) {
    const Vertex& vertex = *vertexIt->second;

    const std::string sourceSerialization =
        Base64::Encode(vertex.Serialization);

    nodesStream << std::format("{},{}", sourceSerialization,
                               (int8_t)vertex.Quality)
                << std::endl;

    for (const auto [move, weakTarget] : vertex.Edges) {
      const std::shared_ptr<const Vertex> target = weakTarget.lock();

      const std::string targetSerialization =
          Base64::Encode(target->Serialization);

      const std::string optimalMove = std::format(
          "{},{},{}", move.PawnId, (size_t)move.UsedCard.Type, move.OffsetId);

      const bool optimal =
          vertex.OptimalMove && move == vertex.OptimalMove.value();

      edgesStream << std::format("{},{},{},{}", sourceSerialization,
                                 targetSerialization, optimalMove, optimal)
                  << std::endl;
    }
  }

  nodesStream.close();
  edgesStream.close();

  std::cout << std::format("Exported nodes: {}", nodesPath.string())
            << std::endl;
  std::cout << std::format("Exported edges: {}", edgesPath.string())
            << std::endl;
}

}  // namespace StateGraph
