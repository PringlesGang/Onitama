#pragma once

#include <memory>
#include <stack>

#include "game/game.h"
#include "strategies/strategy.h"

enum class PrintType {
  Board,
  Data,
};

class GameMaster {
 public:
  GameMaster(std::unique_ptr<Strategy::Strategy> redPlayer,
             std::unique_ptr<Strategy::Strategy> bluePlayer,
             bool repeatCards = false);
  GameMaster(std::unique_ptr<Strategy::Strategy> redPlayer,
             std::unique_ptr<Strategy::Strategy> bluePlayer,
             std::array<Game::Card, CARD_COUNT> cards);

  void Render() const;
  void Update();
  std::optional<Color> IsFinished() const;

  size_t GetRound() const { return Round; }
  const Game::Game& GetGame() const { return GameInstance; }

  PrintType PrintType = PrintType::Board;

 private:
  Game::Game GameInstance;

  std::unique_ptr<Strategy::Strategy> RedPlayer;
  std::unique_ptr<Strategy::Strategy> BluePlayer;

  size_t Round = 1;
  std::stack<Game::Move> MoveHistory;

  void PrintData() const;
  void PrintBoard() const;
};
