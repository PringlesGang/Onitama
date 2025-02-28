#pragma once

#include <iostream>
#include <memory>
#include <stack>

#include "game/game.h"
#include "strategies/strategy.h"

enum class PrintType {
  None,
  Board,
  Data,
  Wins,
};

class GameMaster {
 public:
  GameMaster(const size_t width, const size_t height,
             std::unique_ptr<Strategy::Strategy> redPlayer,
             std::unique_ptr<Strategy::Strategy> bluePlayer,
             bool repeatCards = false);
  GameMaster(const size_t width, const size_t height,
             std::unique_ptr<Strategy::Strategy> redPlayer,
             std::unique_ptr<Strategy::Strategy> bluePlayer,
             std::array<Game::Card, CARD_COUNT> cards);

  void Render(std::ostream& stream = std::cout) const;
  void Update();
  std::optional<Color> IsFinished() const;

  size_t GetRound() const { return Round; }
  const Game::Game& GetGame() const { return GameInstance; }

  PrintType GameMasterPrintType = PrintType::Board;

 private:
  Game::Game GameInstance;

  std::unique_ptr<Strategy::Strategy> RedPlayer;
  std::unique_ptr<Strategy::Strategy> BluePlayer;

  size_t Round = 1;
  std::stack<Game::Move> MoveHistory;

  void PrintData(std::ostream& stream = std::cout) const;
  void PrintBoard(std::ostream& stream = std::cout) const;
};
