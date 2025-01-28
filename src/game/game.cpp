#include "game.h"

#include <random>

bool Move::operator==(const Move& other) const {
  return Source == other.Source && MoveOffset == other.MoveOffset &&
         UsedCard == other.UsedCard;
}

Game::Game(std::array<Card, CARD_COUNT> cards)
    : Cards(cards), Board(), CurrentPlayer(cards[0].GetColor()) {}

Game Game::WithRandomCards(bool repeatCards) {
  std::array<Card, CARD_COUNT> cards;

  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<size_t> randomCard(
      0, (size_t)CardType::CardTypeCount);

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

std::unordered_set<Move> Game::GetValidMoves() const {
  std::vector<Coordinate> pieceLocations =
      Board.GetPieceCoordinates(CurrentPlayer);

  const std::span<Card, HAND_SIZE>& hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;

  std::unordered_set<Move> validMoves;
  for (Coordinate piece : pieceLocations) {
    for (Card card : hand) {
      for (Offset offset : card.GetMoves()) {
        const Move move{
            .Source = piece, .MoveOffset = offset, .UsedCard = card};
        if (IsValidMove(move)) {
          validMoves.insert(move);
        }
      }
    }
  }

  return validMoves;
}

bool Game::IsValidMove(Move move) const {
  if (!move.UsedCard.GetMoves().contains(move.MoveOffset)) return false;

  const std::span<Card, HAND_SIZE> hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return false;

  const std::optional<Tile> sourceTile = Board.GetTile(move.Source);
  if (!sourceTile || !sourceTile->has_value() ||
      sourceTile->value().GetColor() != CurrentPlayer)
    return false;

  const std::optional<Coordinate> dest = move.Source.try_add(move.MoveOffset);
  if (!dest || !Board.OnBoard(dest.value())) return false;

  const std::optional<Tile> destTile = Board.GetTile(*dest);
  if (destTile->has_value() || destTile->value().GetColor() == CurrentPlayer)
    return false;

  return true;
}

bool Game::DoMove(Move move) {
  if (!IsValidMove(move)) return false;

  Board.DoMove(move.Source, move.MoveOffset);

  const std::span<Card, HAND_SIZE> hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;
  const auto usedCardIt = std::find(hand.begin(), hand.end(), move.UsedCard);
  std::swap(SetAsideCard, *usedCardIt);

  return true;
}

std::optional<Color> Game::IsFinished() const { return Board.IsFinished(); }
