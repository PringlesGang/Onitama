#pragma once

#include <memory>

#include "game/game.h"
#include "strategies/strategy.h"

class GameMaster {
 public:
  GameMaster(std::unique_ptr<Strategy::Strategy> redPlayer,
             std::unique_ptr<Strategy::Strategy> bluePlayer);

  void Render();
  void Update();
  std::optional<Color> IsFinished() const;

 private:
  Game::Game GameInstance;

  std::unique_ptr<Strategy::Strategy> RedPlayer;
  std::unique_ptr<Strategy::Strategy> BluePlayer;
};
