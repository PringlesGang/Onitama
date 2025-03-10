#include "game.h"

#include <algorithm>
#include <format>
#include <iostream>
#include <random>
#include <stdexcept>
#include <unordered_set>

#include "../util/base64.h"
#include "../util/parse.h"

namespace Game {

Game::Game(const size_t width, const size_t height,
           std::array<Card, CARD_COUNT> cards)
    : Cards(cards),
      GameBoard(width, height),
      CurrentPlayer(cards[0].GetColor()) {
  SetValidMoves();
}

Game::Game(Board&& board, std::array<Card, CARD_COUNT>&& cards,
           Color&& currentPlayer)
    : GameBoard(std::move(board)),
      Cards(std::move(cards)),
      CurrentPlayer(std::move(currentPlayer)) {
  SetValidMoves();
}

Game::Game(const Game& other)
    : GameBoard(other.GameBoard),
      Cards(other.Cards),
      CurrentPlayer(other.CurrentPlayer),
      ValidMoves(other.ValidMoves),
      HasValidMovesVal(other.HasValidMovesVal) {}

Game::Game(Game&& other)
    : GameBoard(std::move(other.GameBoard)),
      Cards(std::move(other.Cards)),
      CurrentPlayer(std::move(other.CurrentPlayer)),
      ValidMoves(std::move(other.ValidMoves)),
      HasValidMovesVal(std::move(other.HasValidMovesVal)) {}

Game Game::WithRandomCards(const size_t width, const size_t height,
                           const bool repeatCards) {
  std::array<Card, CARD_COUNT> cards;

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<size_t> randomCard(
      0, (size_t)CardType::CardTypeCount - 1);

  for (auto cardIt = cards.begin(); cardIt != cards.end(); cardIt++) {
    do {
      *cardIt = Card(CardType(randomCard(generator)));
    } while (!repeatCards &&
             std::find(cards.begin(), cardIt, *cardIt) != cardIt);
  }

  return Game(width, height, std::move(cards));
}

static constexpr size_t ReadBits(GameSerialization& input,
                                 const size_t length) {
  const size_t result = input.to_ullong() & ((1 << length) - 1);
  input >>= length;
  return result;
}

Game Game::FromSerialization(GameSerialization serialization) {
  // Current player
  constexpr size_t playerSize = 1;
  Color player = ReadBits(serialization, playerSize) ? TopPlayer : ~TopPlayer;

  // Card distribution
  std::array<Card, CARD_COUNT> cards;
  constexpr size_t cardSize =
      std::bit_width((size_t)CardType::CardTypeCount - 1);
  for (Card& card : cards) {
    card = Card((CardType)ReadBits(serialization, cardSize));
  }

  // Board dimensions
  constexpr size_t dimensionsSize = std::bit_width(MAX_DIMENSION);
  const size_t width = ReadBits(serialization, dimensionsSize);
  const size_t height = ReadBits(serialization, dimensionsSize);

  // Pawn locations
  constexpr size_t captured = MAX_DIMENSION * MAX_DIMENSION;
  constexpr size_t coordinateSize = std::bit_width(captured);

  std::vector<Tile> grid(width * height);
  for (size_t pawn = 0; pawn < width * 2; pawn++) {
    const Color pawnColor = pawn < width ? TopPlayer : ~TopPlayer;
    const bool isMaster = pawn % width == 0;

    const size_t offset = ReadBits(serialization, coordinateSize);
    if (offset != captured)
      grid[offset] = Piece{.Team = pawnColor, .Master = isMaster};
  }
  Board board(std::move(grid), std::move(width), std::move(height));

  return Game(std::move(board), std::move(cards), std::move(player));
}

std::optional<GameSerialization> Game::ParseSerialization(
    std::istringstream& stream) {
  std::string string;
  stream >> string;

  if (string.empty()) {
    std::cerr << "No game serialization provided!" << std::endl;
    Parse::Unparse(stream, string);
    return std::nullopt;
  }

  const std::optional<GameSerialization> serialization =
      Base64::Decode<GAME_SERIALIZATION_SIZE>(string);
  if (!serialization) {
    std::cerr << std::format("Invalid base64 game serialization \"{}\"!",
                             string)
              << std::endl;
    Parse::Unparse(stream, string);
    return std::nullopt;
  }

  return serialization;
}

bool Game::operator==(const Game& other) const {
  if (CurrentPlayer != other.CurrentPlayer) return false;

  if (SetAsideCard != other.SetAsideCard) return false;

  if (GetDimensions() != other.GetDimensions()) return false;

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : ~TopPlayer;

    if (MasterCaptured(player) != other.MasterCaptured(player)) return false;

    // Already sorted
    if (GetPawnCoordinates(player) != other.GetPawnCoordinates(player))
      return false;

    if (std::unordered_multiset<Card>(GetHand(player).begin(),
                                      GetHand(player).end()) !=
        std::unordered_multiset<Card>(other.GetHand(player).begin(),
                                      other.GetHand(player).end()))
      return false;
  }

  return true;
}

void Game::SetValidMoves() {
  const std::span<const Card, HAND_SIZE>& hand = GetHand();

  ValidMoves.clear();

  for (size_t pawnId = 0; pawnId < GetPawnCount(); pawnId++) {
    for (Card card : hand) {
      const size_t offsetCount = card.GetMoves().size();
      for (size_t offsetId = 0; offsetId < offsetCount; offsetId++) {
        const Move move{
            .PawnId = pawnId,
            .UsedCard = card,
            .OffsetId = offsetId,
        };

        if (CheckIsValidMove(move)) {
          ValidMoves.emplace_back(std::move(move));
        }
      }
    }
  }

  // No valid moves; add cards
  HasValidMovesVal = !ValidMoves.empty();
  if (!HasValidMovesVal) {
    ValidMoves.reserve(HAND_SIZE);

    for (Card card : GetHand()) {
      ValidMoves.emplace_back(Move{.UsedCard = card});
    }
  }
}

bool Game::CheckIsValidMove(const Move move) const {
  const std::vector<Coordinate>& pawnLocations = GetPawnCoordinates();
  if (move.PawnId >= GetPawnCount()) return false;

  const std::vector<Offset>& offsets = move.UsedCard.GetMoves();
  if (move.OffsetId >= offsets.size()) return false;

  const std::span<const Card, HAND_SIZE> hand = GetHand();
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return false;

  const Offset orientedOffset = offsets[move.OffsetId].Orient(CurrentPlayer);
  const std::optional<const Coordinate> destCoordinate =
      pawnLocations[move.PawnId].try_add(orientedOffset);
  if (!destCoordinate) return false;

  const std::optional<Tile> destTile = GameBoard.GetTile(*destCoordinate);
  if (!destTile || (destTile.value() && (*destTile)->Team == CurrentPlayer))
    return false;

  return true;
}

std::optional<std::string> Game::IsInvalidMove(const Move move) const {
  if (IsValidMove(move)) return std::nullopt;

  const std::vector<Coordinate>& pawnLocations = GetPawnCoordinates();
  if (move.PawnId >= GetPawnCount()) return "Pawn does not exist!";

  const std::vector<Offset>& offsets = move.UsedCard.GetMoves();
  if (move.OffsetId >= offsets.size()) return "Invalid offset number!";

  const std::span<const Card, HAND_SIZE> hand = GetHand(CurrentPlayer);
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return "Used card not in player's hand!";

  const Offset orientedOffset = offsets[move.OffsetId].Orient(CurrentPlayer);
  const std::optional<const Coordinate> destCoordinate =
      pawnLocations[move.PawnId].try_add(orientedOffset);
  if (!GameBoard.OnBoard(destCoordinate)) return "Destination not on board!";

  const Tile destTile = GameBoard.GetTile(*destCoordinate).value();
  if (destTile && destTile->Team == CurrentPlayer)
    return "Cannot capture pawn of the same color!";

  return std::nullopt;
}

void Game::DoMove(const Move move) {
  if (HasValidMoves()) {
    if (!IsValidMove(move)) {
      const std::string message =
          IsInvalidMove(move).value_or("Attempted to perform invalid move!");
      throw std::runtime_error(message);
    }

    const Coordinate startCoordinate = GetPawnCoordinates()[move.PawnId];
    const Offset offset =
        move.UsedCard.GetMoves()[move.OffsetId].Orient(CurrentPlayer);

    GameBoard.DoMove(startCoordinate, offset);
  }

  const std::span<Card, HAND_SIZE> hand = ColorToHand.at(CurrentPlayer);
  const auto usedCardIt = std::find(hand.begin(), hand.end(), move.UsedCard);

  if (usedCardIt == hand.end())
    throw std::runtime_error("Attempted to discard non-owned card!");
  std::swap(SetAsideCard, *usedCardIt);

  CurrentPlayer = ~CurrentPlayer;
  SetValidMoves();
}

std::ostream& operator<<(std::ostream& stream, const Game& game) {
  const std::span<const Card, HAND_SIZE> topHand = game.GetHand(TopPlayer);
  const std::span<const Card, HAND_SIZE> bottomHand = game.GetHand(~TopPlayer);

  std::string cardNumberString = "";
  for (size_t card = 0; card < HAND_SIZE; card++) {
    cardNumberString += std::format("Card {} ", card);
  }

  if (game.CurrentPlayer == TopPlayer) stream << cardNumberString << std::endl;
  game.StreamHand(stream, topHand, true);

  size_t pawnIndex = 0;
  for (size_t row = 0; row < game.GameBoard.GetDimensions().second; row++) {
    game.GameBoard.StreamPlayerRow(stream, game.CurrentPlayer, row, pawnIndex);
    stream << "  ";

    const bool reverse = game.CurrentPlayer == TopPlayer;
    const int8_t sign = reverse ? -1 : 1;
    game.SetAsideCard.StreamRow(stream, sign * (row - 2),
                                game.CurrentPlayer == TopPlayer)
        << std::endl;
  }

  stream << std::endl;
  if (game.CurrentPlayer != TopPlayer) stream << cardNumberString << std::endl;
  game.StreamHand(stream, bottomHand, false);

  const std::optional<Color> finished = game.IsFinished();
  if (finished) {
    stream << "Winner: " << *finished << std::endl;
  } else {
    stream << "Current player: " << game.CurrentPlayer << std::endl;
  }

  return stream << std::endl;
}

std::ostream& Game::StreamHand(std::ostream& stream,
                               const std::span<const Card, HAND_SIZE> hand,
                               const bool rotate) const {
  const uint8_t sign = rotate ? -1 : 1;
  for (int8_t row = -2 * sign; abs(row) <= 2; row += sign) {
    for (Card card : hand) {
      card.StreamRow(stream, row, rotate) << "  ";
    }

    stream << std::endl;
  }

  return stream << std::endl;
}

static void AddBits(GameSerialization& input, const size_t bits,
                    size_t& inputSize, const size_t bitsSize) {
  input |= bits << inputSize;
  inputSize += bitsSize;
}

GameSerialization Game::Serialize() const {
  const auto [width, height] = GetDimensions();
  if (width > MAX_DIMENSION || height > MAX_DIMENSION) {
    const std::string msg = std::format(
        "Tried to serialize board of size {}x{} when max size is {}x{}!", width,
        height, MAX_DIMENSION, MAX_DIMENSION);
    throw std::runtime_error(msg);
  }

  GameSerialization serialization;
  size_t size = 0;

  // Current player
  constexpr size_t playerSize = 1;
  const size_t player = CurrentPlayer == TopPlayer;
  AddBits(serialization, player, size, playerSize);

  // Card distribution
  constexpr size_t cardSize =
      std::bit_width((size_t)CardType::CardTypeCount - 1);
  const std::span<const Card, CARD_COUNT> cards = GetCards();
  for (const Card card : cards) {
    AddBits(serialization, (size_t)card.Type, size, cardSize);
  }

  // Board dimensions
  constexpr size_t dimensionSize = std::bit_width(MAX_DIMENSION);
  AddBits(serialization, width, size, dimensionSize);
  AddBits(serialization, height, size, dimensionSize);

  // Pawn locations
  // Extra bit value representation for 'captured'
  constexpr size_t captured = MAX_DIMENSION * MAX_DIMENSION;
  constexpr size_t coordinateSize = std::bit_width(captured);

  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : ~TopPlayer;

    if (MasterCaptured(player)) {
      AddBits(serialization, captured, size, coordinateSize);
    }

    const std::vector<Coordinate>& locations = GetPawnCoordinates(player);
    for (const Coordinate location : locations) {
      const size_t offset = location.x + location.y * width;
      AddBits(serialization, offset, size, coordinateSize);
    }

    // Serialize captured pawns
    for (size_t i = locations.size() + MasterCaptured(); i < width; i++) {
      AddBits(serialization, captured, size, coordinateSize);
    }
  }

  return serialization;
}

}  // namespace Game

size_t std::hash<Game::Game>::operator()(
    const Game::Game& game) const noexcept {
  // Cards
  size_t hash = (size_t)game.GetSetAsideCard().Type;

  for (const Game::Card card : game.GetHand()) {
    hash ^= (size_t)card.Type;
  }

  // Pawn locations
  const size_t width = game.GetDimensions().first;
  for (size_t playerId = 0; playerId < 2; playerId++) {
    const Color player = playerId == 0 ? TopPlayer : ~TopPlayer;
    for (const Coordinate coordinate : game.GetPawnCoordinates(player)) {
      hash ^= coordinate.x + width * coordinate.y;
    }
  }

  return hash;
}
