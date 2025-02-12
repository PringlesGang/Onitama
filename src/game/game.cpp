#include "game.h"

#include <algorithm>
#include <format>
#include <random>
#include <stdexcept>

namespace Game {

Game::Game(std::array<Card, CARD_COUNT> cards)
    : Cards(cards), GameBoard(), CurrentPlayer(cards[0].GetColor()) {
  SetValidMoves();
}

Game::Game(const Game& other)
    : GameBoard(other.GameBoard),
      Cards(other.Cards),
      CurrentPlayer(other.CurrentPlayer),
      ValidMoves(other.ValidMoves) {}

Game::Game(Game&& other)
    : GameBoard(std::move(other.GameBoard)),
      Cards(std::move(other.Cards)),
      CurrentPlayer(std::move(other.CurrentPlayer)),
      ValidMoves(std::move(other.ValidMoves)) {}

Game& Game::operator=(const Game& other) {
  GameBoard = other.GameBoard;
  Cards = other.Cards;
  CurrentPlayer = other.CurrentPlayer;
  ValidMoves = other.ValidMoves;

  return *this;
}

Game& Game::operator=(Game&& other) {
  GameBoard = std::move(other.GameBoard);
  Cards = std::move(other.Cards);
  CurrentPlayer = std::move(other.CurrentPlayer);
  ValidMoves = std::move(other.ValidMoves);

  return *this;
}

Game Game::WithRandomCards(const bool repeatCards) {
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

  return Game(cards);
}

std::span<const Card, HAND_SIZE> Game::GetHand(const Color color) const {
  switch (color) {
    case Color::Blue:
      return BlueHand;
    case Color::Red:
      return RedHand;

    default:
      const size_t colorNum = (size_t)color;
      throw std::runtime_error(std::format("Invalid color {}", colorNum));
  }
}

std::span<const Card, HAND_SIZE> Game::GetCurrentHand() const {
  return GetHand(CurrentPlayer);
}

void Game::SetValidMoves() {
  const std::vector<Coordinate>& pieceLocations =
      GameBoard.GetPieceCoordinates(CurrentPlayer);

  const std::span<const Card, HAND_SIZE>& hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;

  ValidMoves.clear();

  for (size_t pawnId = 0; pawnId < pieceLocations.size(); pawnId++) {
    for (Card card : hand) {
      const size_t offsetCount = card.GetMoves().size();
      for (size_t offsetId = 0; offsetId < offsetCount; offsetId++) {
        const Move move{
            .PawnId = pawnId,
            .UsedCard = card,
            .OffsetId = offsetId,
        };

        if (IsValidMove(move)) {
          ValidMoves.emplace(std::move(move));
        }
      }
    }
  }
}

bool Game::IsValidMove(const Move move) const {
  const std::vector<Coordinate>& pawnLocations =
      GameBoard.GetPieceCoordinates(CurrentPlayer);
  if (move.PawnId >= pawnLocations.size()) return false;

  const std::vector<Offset> offsets = move.UsedCard.GetMoves();
  if (move.OffsetId >= offsets.size()) return false;

  const std::span<const Card, HAND_SIZE> hand = GetHand(CurrentPlayer);
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return false;

  const Offset orientedOffset = offsets[move.OffsetId].Orient(CurrentPlayer);
  const std::optional<const Coordinate> destCoordinate =
      pawnLocations[move.PawnId].try_add(orientedOffset);
  if (!GameBoard.OnBoard(destCoordinate)) return false;

  const Tile destTile = GameBoard.GetTile(*destCoordinate).value();
  if (destTile && destTile->Team == CurrentPlayer) return false;

  return true;
}

std::optional<std::string> Game::IsInvalidMove(const Move move) const {
  if (IsValidMoveFast(move)) return std::nullopt;

  const std::vector<Coordinate>& pawnLocations =
      GameBoard.GetPieceCoordinates(CurrentPlayer);
  if (move.PawnId >= pawnLocations.size()) return "Pawn does not exist!";

  const std::vector<Offset> offsets = move.UsedCard.GetMoves();
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

bool Game::IsValidMoveFast(const Move move) const {
  return ValidMoves.contains(move);
}

bool Game::DoMove(const Move move) {
  const size_t validMoveCount = GetValidMoves().size();

  if (validMoveCount > 0) {
    if (!IsValidMoveFast(move)) return false;

    const Coordinate startCoordinate =
        GameBoard.GetPieceCoordinates(CurrentPlayer)[move.PawnId];
    const Offset offset =
        move.UsedCard.GetMoves()[move.OffsetId].Orient(CurrentPlayer);

    GameBoard.DoMove(startCoordinate, offset);
  }

  const std::span<Card, HAND_SIZE> hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;
  const auto usedCardIt = std::find(hand.begin(), hand.end(), move.UsedCard);

  if (usedCardIt == hand.end()) return false;
  std::swap(SetAsideCard, *usedCardIt);

  CurrentPlayer = ~CurrentPlayer;
  SetValidMoves();

  return true;
}

std::ostream& operator<<(std::ostream& stream, const Game& game) {
  const std::span<Card, HAND_SIZE> topHand =
      TopPlayer == Color::Red ? game.RedHand : game.BlueHand;
  const std::span<Card, HAND_SIZE> bottomHand =
      TopPlayer != Color::Red ? game.RedHand : game.BlueHand;

  std::string cardNumberString = "";
  for (size_t card = 0; card < HAND_SIZE; card++) {
    cardNumberString += std::format("Card {} ", card);
  }

  if (game.CurrentPlayer == TopPlayer) stream << cardNumberString << std::endl;
  game.StreamHand(stream, topHand, true);

  size_t pawnIndex = 0;
  for (size_t row = 0; row < BOARD_DIMENSIONS; row++) {
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

}  // namespace Game
