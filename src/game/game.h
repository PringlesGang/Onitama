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
  Game(const size_t width, const size_t height,
       std::array<Card, CARD_COUNT> cards);

  Game(const Game& other);
  Game(Game&& other);

  static Game WithRandomCards(const size_t width, const size_t height,
                              const bool repeatCards = false);

  const Board& GetBoard() const { return GameBoard; }
  std::span<const Card, CARD_COUNT> GetCards() const { return Cards; }
  std::span<const Card, HAND_SIZE> GetHand(const Color color) const;
  std::span<const Card, HAND_SIZE> GetCurrentHand() const;
  Card GetSetAsideCard() const { return SetAsideCard; }
  Color GetCurrentPlayer() const { return CurrentPlayer; }

  size_t GetPawnCount() const { return GetPawnCount(CurrentPlayer); }
  size_t GetPawnCount(const Color color) const {
    return GameBoard.GetPieceCoordinates(color).size();
  }

  const std::unordered_set<Move>& GetValidMoves() const { return ValidMoves; };
  std::optional<std::string> IsInvalidMove(const Move move) const;
  bool IsValidMoveFast(const Move move) const;
  std::optional<Color> IsFinished() const { return GameBoard.IsFinished(); }
  bool DoMove(const Move move);

  friend std::ostream& operator<<(std::ostream& stream, const Game& game);

 private:
  bool IsValidMove(const Move move) const;
  void SetValidMoves();

  Board GameBoard;
  std::array<Card, CARD_COUNT> Cards;
  Color CurrentPlayer;

  Card& SetAsideCard = Cards[0];
  const std::span<Card, HAND_SIZE> RedHand =
      std::span<Card, HAND_SIZE>(&Cards[1], HAND_SIZE);
  const std::span<Card, HAND_SIZE> BlueHand =
      std::span<Card, HAND_SIZE>(&Cards[HAND_SIZE + 1], HAND_SIZE);

  std::unordered_set<Move> ValidMoves;

  std::ostream& StreamHand(std::ostream& stream,
                           const std::span<const Card, HAND_SIZE> hand,
                           const bool rotate = false) const;
};

}  // namespace Game
