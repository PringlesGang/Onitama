#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <unordered_set>

#include "../constants.h"
#include "../util/color.h"
#include "board.h"
#include "card.h"
#include "move.h"

namespace Game {

class Game {
 public:
  Game(std::array<Card, CARD_COUNT> cards);
  Game(const Game& other);

  static Game WithRandomCards(const bool repeatCards = false);

  const Board& GetBoard() const { return Board; }
  std::span<const Card, CARD_COUNT> GetCards() const { return Cards; }
  std::span<const Card, HAND_SIZE> GetHand(const Color color) const;
  std::span<const Card, HAND_SIZE> GetCurrentHand() const;
  Card GetSetAsideCard() const { return SetAsideCard; }
  Color GetCurrentPlayer() const { return CurrentPlayer; }

  std::unordered_set<Move> GetValidMoves() const;
  std::optional<std::string> IsInvalidMove(const Move move) const;
  std::optional<Color> IsFinished() const { return Board.IsFinished(); }
  bool DoMove(const Move move);

  friend std::ostream& operator<<(std::ostream& stream, const Game& game);

 private:
  Board Board;
  std::array<Card, CARD_COUNT> Cards;
  Color CurrentPlayer;

  Card& SetAsideCard = Cards[0];
  const std::span<Card, HAND_SIZE> RedHand =
      std::span<Card, HAND_SIZE>(&Cards[1], HAND_SIZE);
  const std::span<Card, HAND_SIZE> BlueHand =
      std::span<Card, HAND_SIZE>(&Cards[HAND_SIZE + 1], HAND_SIZE);

  std::ostream& StreamHand(std::ostream& stream,
                           const std::span<const Card, HAND_SIZE> hand,
                           const bool rotate = false) const;
};

}  // namespace Game
