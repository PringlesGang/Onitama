#pragma once

#include <array>
#include <optional>
#include <ostream>
#include <span>
#include <string>
#include <unordered_map>

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
  Game(Board&& board, std::array<Card, CARD_COUNT>&& cards,
       Color&& currentPlayer);

  Game(const Game& other);
  Game(Game&& other);

  static Game WithRandomCards(const size_t width, const size_t height,
                              const bool repeatCards = false);
  static Game FromHash(const size_t hash,
                       const std::array<Card, CARD_COUNT>& cards,
                       const size_t width, const size_t height);

  bool operator==(const Game& other) const;

  const Board& GetBoard() const { return GameBoard; }
  std::span<const Card, CARD_COUNT> GetCards() const { return Cards; }
  std::span<const Card, HAND_SIZE> GetHand(const Color color) const {
    return ColorToHand.at(color);
  }
  std::span<const Card, HAND_SIZE> GetHand() const {
    return GetHand(CurrentPlayer);
  }
  Card GetSetAsideCard() const { return SetAsideCard; }
  Color GetCurrentPlayer() const { return CurrentPlayer; }
  std::pair<size_t, size_t> GetDimensions() const {
    return GameBoard.GetDimensions();
  }

  const std::vector<Coordinate>& GetPawnCoordinates() const {
    return GetPawnCoordinates(CurrentPlayer);
  }
  const std::vector<Coordinate>& GetPawnCoordinates(Color color) const {
    return GameBoard.GetPawnCoordinates(color);
  }

  bool MasterCaptured() const { return MasterCaptured(CurrentPlayer); }
  bool MasterCaptured(Color color) const {
    return GameBoard.MasterCaptured(color);
  }

  size_t GetPawnCount() const { return GetPawnCount(CurrentPlayer); }
  size_t GetPawnCount(const Color color) const {
    return GameBoard.GetPawnCoordinates(color).size();
  }

  const std::vector<Move>& GetValidMoves() const { return ValidMoves; };
  bool HasValidMoves() const { return HasValidMovesVal; }
  std::optional<std::string> IsInvalidMove(const Move move) const;
  bool IsValidMove(const Move move) const {
    return std::find(ValidMoves.begin(), ValidMoves.end(), move) !=
           ValidMoves.end();
  }
  std::optional<Color> IsFinished() const { return GameBoard.IsFinished(); }
  void DoMove(const Move move);

  friend std::ostream& operator<<(std::ostream& stream, const Game& game);

 private:
  bool CheckIsValidMove(const Move move) const;
  void SetValidMoves();

  Board GameBoard;
  std::array<Card, CARD_COUNT> Cards;
  Color CurrentPlayer;

  Card& SetAsideCard = Cards[0];
  const std::unordered_map<Color, std::span<Card, HAND_SIZE>> ColorToHand = {
      {Color::Red, std::span<Card, HAND_SIZE>(&Cards[1], HAND_SIZE)},
      {Color::Blue,
       std::span<Card, HAND_SIZE>(&Cards[HAND_SIZE + 1], HAND_SIZE)},
  };

  std::vector<Move> ValidMoves;
  bool HasValidMovesVal;

  std::ostream& StreamHand(std::ostream& stream,
                           const std::span<const Card, HAND_SIZE> hand,
                           const bool rotate = false) const;
};

}  // namespace Game

template <>
struct std::hash<Game::Game> {
  size_t operator()(const Game::Game& game) const noexcept;
};
