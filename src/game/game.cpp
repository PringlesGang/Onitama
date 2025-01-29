#include "game.h"

#include <format>
#include <random>
#include <stdexcept>

namespace Game {

Game::Game(std::array<Card, CARD_COUNT> cards)
    : Cards(cards), Board(), CurrentPlayer(cards[0].GetColor()) {}

Game Game::WithRandomCards(bool repeatCards) {
  std::array<Card, CARD_COUNT> cards;

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<size_t> randomCard(
      0, (size_t)CardType::CardTypeCount - 1);

  for (auto cardIt = cards.begin(); cardIt != cards.end(); cardIt++) {
    do {
      *cardIt = Card(CardType(randomCard(generator)));
    } while (!repeatCards &&
             std::find_if(cards.begin(), cardIt, [cardIt](Card& card) {
               return card.Type == cardIt->Type;
             }) != cardIt);
  }

  return Game(cards);
}

std::span<const Card, HAND_SIZE> Game::GetHand(Color color) const {
  switch (color) {
    case Color::Blue:
      return BlueHand;
    case Color::Red:
      return RedHand;

    default:
      size_t colorNum = (size_t)color;
      throw std::runtime_error(
          std::vformat("Invalid color {}", std::make_format_args(colorNum)));
  }
}

std::span<const Card, HAND_SIZE> Game::GetCurrentHand() const {
  return GetHand(CurrentPlayer);
}

Card Game::GetSetAsideCard() const { return SetAsideCard; }

Color Game::GetCurrentPlayer() const { return CurrentPlayer; }

std::unordered_set<Move> Game::GetValidMoves() const {
  std::vector<Coordinate> pieceLocations =
      Board.GetPieceCoordinates(CurrentPlayer);

  const std::span<Card, HAND_SIZE>& hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;

  std::unordered_set<Move> validMoves;
  for (size_t pawnId = 0; pawnId < pieceLocations.size(); pawnId++) {
    for (Card card : hand) {
      const size_t offsetCount = card.GetMoves().size();
      for (size_t offsetId = 0; offsetId < offsetCount; offsetId++) {
        const Move move{
            .PawnId = pawnId,
            .UsedCard = card,
            .OffsetId = offsetId,
        };

        if (!IsInvalidMove(move)) {
          validMoves.insert(move);
        }
      }
    }
  }

  return validMoves;
}

std::optional<std::string> Game::IsInvalidMove(Move move) const {
  const std::vector<Coordinate> pawnLocations =
      Board.GetPieceCoordinates(CurrentPlayer);
  if (move.PawnId >= pawnLocations.size()) return "Pawn does not exist!";

  const std::vector<Offset> offsets = move.UsedCard.GetMoves();
  if (move.OffsetId >= offsets.size()) return "Invalid offset number!";

  const std::span<const Card, HAND_SIZE> hand = GetHand(CurrentPlayer);
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return "Used card not in player's hand!";

  const std::vector<Coordinate> pawnCoordinates =
      Board.GetPieceCoordinates(CurrentPlayer);
  if (move.PawnId >= pawnCoordinates.size()) return "Pawn does not exist!";

  const Offset offset = offsets[move.OffsetId];
  const std::optional<Coordinate> destCoordinate =
      pawnCoordinates[move.PawnId].try_add(offset);
  if (!destCoordinate || !Board.OnBoard(*destCoordinate))
    return "Destination not on board!";

  const Tile destTile = Board.GetTile(*destCoordinate).value();
  if (destTile->GetColor() == CurrentPlayer)
    return "Cannot capture pawn of the same color!";

  return std::optional<std::string>();
}

bool Game::DoMove(Move move) {
  if (IsInvalidMove(move)) return false;

  const Coordinate startCoordinate =
      Board.GetPieceCoordinates(CurrentPlayer)[move.PawnId];
  const Offset offset = move.UsedCard.GetMoves()[move.OffsetId];

  Board.DoMove(startCoordinate, offset);

  const std::span<Card, HAND_SIZE> hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;
  const auto usedCardIt = std::find(hand.begin(), hand.end(), move.UsedCard);
  std::swap(SetAsideCard, *usedCardIt);

  return true;
}

std::optional<Color> Game::IsFinished() const { return Board.IsFinished(); }

std::ostream& operator<<(std::ostream& stream, const Game& game) {
  const std::span<Card, HAND_SIZE> topHand =
      TopPlayer == Color::Red ? game.RedHand : game.BlueHand;
  const std::span<Card, HAND_SIZE> bottomHand =
      TopPlayer != Color::Red ? game.RedHand : game.BlueHand;

  game.StreamHand(stream, topHand, true);

  size_t pawnIndex = 0;
  for (size_t row = 0; row < BOARD_DIMENSIONS; row++) {
    game.Board.StreamPlayerRow(stream, game.CurrentPlayer, row, pawnIndex);
    stream << "  ";

    const bool reverse = game.CurrentPlayer == TopPlayer;
    const int8_t sign = reverse ? -1 : 1;
    game.SetAsideCard.StreamRow(stream, sign * (row - 2),
                                game.CurrentPlayer == TopPlayer)
        << std::endl;
  }

  stream << std::endl;
  game.StreamHand(stream, bottomHand, false);
  stream << "Current player: " << game.CurrentPlayer << std::endl;
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
