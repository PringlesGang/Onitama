#include "stateGraph.h"

#include <format>
#include <fstream>
#include <iostream>

#include "../util/base64.h"

namespace StateGraph {

Vertex::Vertex(const Game::Game& game)
    : Serialization(game.Serialize()),
      Quality(game.IsFinished() ? std::optional<WinState>(WinState::Lose)
                                : std::nullopt) {}

Vertex::Vertex(Game::GameSerialization serialization,
               std::optional<WinState> quality)
    : Serialization(serialization), Quality(quality) {}

void Vertex::SetOptimalMove(const Game::Move move) {
  for (std::shared_ptr<Edge> edge : Edges) {
    const bool isOptimalMove = edge->Move == move;

    if (edge->Optimal.has_value() || isOptimalMove) {
      edge->Optimal = isOptimalMove;
    }
  }
}

std::optional<Game::Move> Vertex::GetOptimalMove() const {
  const auto optimalMoveIt = std::find_if(
      Edges.begin(), Edges.end(),
      [](std::shared_ptr<Edge> edge) -> bool { return edge->IsOptimal(); });

  return optimalMoveIt == Edges.end()
             ? std::nullopt
             : std::optional<Game::Move>((*optimalMoveIt)->Move);
}

std::optional<std::shared_ptr<Edge>> Vertex::GetEdge(const Game::Move move) {
  const auto correspondingEdge = [move](std::shared_ptr<Edge> edge) {
    return edge->Move == move ? true : false;
  };

  const auto foundEdge =
      std::find_if(Edges.begin(), Edges.end(), correspondingEdge);
  return foundEdge == Edges.end()
             ? std::nullopt
             : std::optional<std::shared_ptr<Edge>>(*foundEdge);
}

std::optional<std::shared_ptr<Edge>> Vertex::GetEdge(
    const Game::GameSerialization game) {
  const auto correspondingEdge = [game](std::shared_ptr<Edge> edge) {
    const std::shared_ptr<Vertex> vertex = edge->Target.lock();
    return vertex != nullptr && vertex->Serialization == game;
  };

  const auto foundEdge =
      std::find_if(Edges.begin(), Edges.end(), correspondingEdge);
  return foundEdge == Edges.end()
             ? std::nullopt
             : std::optional<std::shared_ptr<Edge>>(*foundEdge);
}

static bool CompareCoordinates(const Game::Game& first,
                               const Game::Game& second,
                               const Color firstPlayer,
                               const Color secondPlayer) {
  const bool masterCaptured = first.MasterCaptured(firstPlayer);
  if (masterCaptured != second.MasterCaptured(secondPlayer)) return false;

  const std::vector<Coordinate>& firstCoordinates =
      first.GetPawnCoordinates(firstPlayer);
  const std::vector<Coordinate>& secondCoordinates =
      second.GetPawnCoordinates(secondPlayer);

  if (firstCoordinates.size() != secondCoordinates.size()) return false;
  if (firstCoordinates.empty()) return true;

  const bool orient = firstPlayer != secondPlayer;
  if (orient) {
    const auto [width, height] = second.GetDimensions();
    const auto orientCoord = [width, height](Coordinate coordinate) {
      return Coordinate(width - coordinate.x - 1, height - coordinate.y - 1);
    };
    const auto orientEqual = [orientCoord](Coordinate first,
                                           Coordinate second) {
      return first == orientCoord(second);
    };

    if (!masterCaptured &&
        !orientEqual(firstCoordinates[0], secondCoordinates[0])) {
      return false;
    }

    return std::equal(firstCoordinates.begin() + !masterCaptured,
                      firstCoordinates.end(), secondCoordinates.rbegin(),
                      orientEqual);
  } else {
    return firstCoordinates == secondCoordinates;
  }
}

Edge::Edge(std::weak_ptr<Vertex> source, std::weak_ptr<Vertex> target,
           Game::Move move, std::optional<bool> optimal)
    : Source(source), Target(target), Move(move), Optimal(optimal) {}

bool EqualTo::operator()(const Game::Game& first,
                         const Game::Game& second) const noexcept {
  if (!UseSymmetries) return first == second;

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
        second.GetHand(secondPlayer);

    if (!(firstHand[0] == secondHand[0] && firstHand[1] == secondHand[1] ||
          firstHand[0] == secondHand[1] && firstHand[1] == secondHand[0])) {
      return false;
    }

    if (!CompareCoordinates(first, second, firstPlayer, secondPlayer))
      return false;
  }

  return true;
}

size_t Hash::operator()(const Game::Game& game) const noexcept {
  // Cards
  size_t hash = (size_t)game.GetSetAsideCard().Type;

  for (const Game::Card card : game.GetHand(game.GetCurrentPlayer())) {
    hash ^= (size_t)card.Type;
  }

  // Pawn locations
  const auto [width, height] = game.GetDimensions();
  const size_t boardEnd = width * height - 1;

  const bool orient = TopPlayer == game.GetCurrentPlayer();

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : BottomPlayer;
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
  return Vertices.contains(game) ? std::optional(Vertices.at(game))
                                 : std::nullopt;
};

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

  std::optional<WinState> quality;
  try {
    const int8_t qualityVal = std::stoi(qualityValString);
    quality = (WinState)qualityVal;
  } catch (std::invalid_argument e) {
    quality = std::nullopt;
  }

  return Vertex(std::move(serialization), std::move(quality));
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
  const std::weak_ptr<Vertex> target = Vertices.at(targetGame);

  // Move
  const std::optional<Game::Move> move = ParseMove(string);
  if (!move) return std::nullopt;

  // Optimal move
  std::string optimalString;
  std::getline(string, optimalString);

  std::optional<bool> optimal = std::nullopt;
  if (optimalString == "true") {
    optimal = true;
  } else if (optimalString == "false") {
    optimal = false;
  }

  return Edge(source, target, move.value(), optimal);
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
    source->Edges.emplace_back(std::make_shared<Edge>(edge.value()));
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
  nodesStream << "Id, Quality, image" << std::endl;
  edgesStream << "Source, Target, Pawn, Card, Offset, Optimal" << std::endl;

  for (auto vertexIt = Vertices.begin(); vertexIt != Vertices.end();
       vertexIt++) {
    const Vertex& vertex = *vertexIt->second;

    const std::string sourceSerialization =
        Base64::Encode(vertex.Serialization);
    const std::string quality =
        vertex.Quality.transform(to_string).value_or("Unknown");

    nodesStream << std::format("{},{},{}.bmp", sourceSerialization, quality,
                               sourceSerialization)
                << std::endl;

    for (const std::shared_ptr<Edge> edge : vertex.Edges) {
      const std::shared_ptr<const Vertex> target = edge->Target.lock();
      if (target == nullptr) continue;
      const Game::Move move = edge->Move;

      const std::string targetSerialization =
          Base64::Encode(target->Serialization);

      const std::string optimalMove = std::format(
          "{},{},{}", move.PawnId, (size_t)move.UsedCard.Type, move.OffsetId);

      edgesStream << std::format("{},{},{},{}", sourceSerialization,
                                 targetSerialization, optimalMove,
                                 edge->IsOptimal())
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

void Graph::ExportImages(const std::filesystem::path& imagesPath) const {
  std::cout << std::format("Exporting images: {}", imagesPath.string())
            << std::endl;

  for (const auto [game, vertex] : Vertices) {
    game.ExportImage(imagesPath);
  }
}

size_t Graph::GetEdgeCount() const {
  size_t count = 0;

  std::for_each(
      Vertices.begin(), Vertices.end(),
      [&count](std::pair<Game::Game, std::shared_ptr<const Vertex>> vertex) {
        count += vertex.second->Edges.size();
      });

  return count;
}

}  // namespace StateGraph
