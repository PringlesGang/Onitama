#pragma once

#include "board.h"

constexpr size_t HAND_SIZE = 2;
constexpr size_t CARD_COUNT = HAND_SIZE * 2 + 1;

class Game {
 public:
  Game(std::array<Card, CARD_COUNT> cards);

  static Game WithRandomCards(bool repeatCards = false);

  std::optional<Color> IsFinished() const;

 private:
  Board Board;
  std::array<Card, CARD_COUNT> Cards;
  Color CurrentPlayer;
};
