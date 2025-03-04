#include "game.h"

#include <algorithm>
#include <format>
#include <random>
#include <stdexcept>

namespace Game {

Game::Game(const size_t width, const size_t height,
           std::array<Card, CARD_COUNT> cards)
    : Cards(cards),
      GameBoard(width, height),
      CurrentPlayer(cards[0].GetColor()) {
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

bool Game::operator==(const Game& other) const {
  return std::hash<Game>{}(*this) == std::hash<Game>{}(other);
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

}  // namespace Game
