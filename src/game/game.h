#pragma once

#include <unordered_set>

#include "board.h"

constexpr size_t HAND_SIZE = 2;
constexpr size_t CARD_COUNT = HAND_SIZE * 2 + 1;

struct Move {
  Coordinate Source;
  Offset MoveOffset;
  Card UsedCard;

  bool operator==(const Move& move) const;
};

template <>
struct std::hash<Move> {
  size_t operator()(const Move& move) const noexcept {
    return ((move.Source.x + move.MoveOffset.dx) << 16) ^
           ((move.Source.y + move.MoveOffset.dy) << 8) ^
           (size_t)move.UsedCard.Type;
  }
};

class Game {
 public:
  Game(std::array<Card, CARD_COUNT> cards);

  static Game WithRandomCards(bool repeatCards = false);

  std::unordered_set<Move> GetValidMoves() const;
  bool IsValidMove(Move move) const;
  std::optional<Color> IsFinished() const;
  bool DoMove(Move move);

 private:
  Board Board;
  std::array<Card, CARD_COUNT> Cards;
  Color CurrentPlayer;

  Card& SetAsideCard = Cards[0];
  std::span<Card, HAND_SIZE> RedHand =
      std::span<Card, HAND_SIZE>(&Cards[1], HAND_SIZE);
  std::span<Card, HAND_SIZE> BlueHand =
      std::span<Card, HAND_SIZE>(&Cards[HAND_SIZE + 1], HAND_SIZE);
};
