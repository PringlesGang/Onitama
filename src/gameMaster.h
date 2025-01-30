#pragma once

#include <memory>

#include "game/game.h"
#include "strategies/strategy.h"

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

  size_t GetRound() const { return round; }
  const Game::Game& GetGame() const { return GameInstance; }

 private:
  Game::Game GameInstance;

  std::unique_ptr<Strategy::Strategy> RedPlayer;
  std::unique_ptr<Strategy::Strategy> BluePlayer;

  size_t round = 1;
};
