#include "game.h"

#include <random>

bool Move::operator==(const Move& other) const {
  return Source == other.Source && OrientedOffset == other.OrientedOffset &&
         UsedCard == other.UsedCard;
}

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
            .Source = piece,
            .OrientedOffset = CurrentPlayer == TopPlayer ? -offset : offset,
            .UsedCard = card};

        if (IsValidMove(move)) {
          validMoves.insert(move);
        }
      }
    }
  }

  return validMoves;
}

bool Game::IsValidMove(Move move) const {
  const Offset nonOrientedOffset =
      CurrentPlayer == TopPlayer ? -move.OrientedOffset : move.OrientedOffset;

  if (!move.UsedCard.GetMoves().contains(nonOrientedOffset)) return false;

  const std::span<Card, HAND_SIZE> hand =
      CurrentPlayer == Color::Red ? RedHand : BlueHand;
  if (std::find(hand.begin(), hand.end(), move.UsedCard) == hand.end())
    return false;

  const std::optional<Tile> sourceTile = Board.GetTile(move.Source);
  if (!sourceTile || !sourceTile->has_value() ||
      sourceTile->value().GetColor() != CurrentPlayer)
    return false;

  const std::optional<Coordinate> dest =
      move.Source.try_add(move.OrientedOffset);
  if (!dest || !Board.OnBoard(dest.value())) return false;

  const std::optional<Tile> destTile = Board.GetTile(*dest);
  if (destTile->has_value() || destTile->value().GetColor() == CurrentPlayer)
    return false;

  return true;
}

bool Game::DoMove(Move move) {
  if (!IsValidMove(move)) return false;

  Board.DoMove(move.Source, move.OrientedOffset);

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

  return stream << topHand
                << std::pair<const Board&, Card>(game.Board, game.SetAsideCard)
                << bottomHand << "Current player: "
                << (game.CurrentPlayer == Color::Red ? "Red" : "Blue")
                << std::endl
                << std::endl;
}

std::ostream& operator<<(std::ostream& stream,
                         const std::span<const Card, HAND_SIZE> cards) {
  for (int8_t row = -2; row <= 2; row++) {
    for (Card card : cards) {
      const std::unordered_set<Offset> offsets = card.GetMoves();

      for (int8_t column = -2; column <= 2; column++) {
        if (row == 0 && column == 0) {
          stream << 'O';
        } else if (offsets.contains(Offset{.dx = column, .dy = row})) {
          stream << 'X';
        } else {
          stream << '.';
        }
      }

      stream << "  ";
    }

    stream << std::endl;
  }

  return stream << std::endl;
}

std::ostream& operator<<(std::ostream& stream,
                         const std::pair<const Board&, Card> boardAndCard) {
  const Board& board = boardAndCard.first;
  const std::unordered_set<Offset> offsets = boardAndCard.second.GetMoves();

  for (int8_t row = -2; row <= 2; row++) {
    for (size_t column = 0; column < BOARD_DIMENSIONS; column++) {
      stream << board.GetTile(Coordinate(column, row + 2)).value();
    }

    stream << "  ";

    for (int8_t column = -2; column <= 2; column++) {
      if (row == 0 && column == 0) {
        stream << 'O';
      } else if (offsets.contains(Offset{.dx = column, .dy = row})) {
        stream << 'X';
      } else {
        stream << '.';
      }
    }

    stream << std::endl;
  }

  return stream << std::endl;
}
