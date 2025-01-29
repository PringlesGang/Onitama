#pragma once

#include <array>
#include <ostream>
#include <span>
#include <unordered_set>

#include "../util/color.h"
#include "board.h"
#include "card.h"
#include "move.h"

constexpr size_t HAND_SIZE = 2;
constexpr size_t CARD_COUNT = HAND_SIZE * 2 + 1;

class Game {
 public:
  Game(std::array<Card, CARD_COUNT> cards);

  static Game WithRandomCards(bool repeatCards = false);

  std::unordered_set<Move> GetValidMoves() const;
  bool IsValidMove(Move move) const;
  std::optional<Color> IsFinished() const;
  bool DoMove(Move move);

  friend std::ostream& operator<<(std::ostream& stream, const Game& game);

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

std::ostream& operator<<(std::ostream& stream,
                         const std::span<const Card, HAND_SIZE> cards);
std::ostream& operator<<(std::ostream& stream,
                         const std::pair<const Board&, Card> boardAndCard);
