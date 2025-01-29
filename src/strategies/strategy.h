#pragma once

#include "../game/game.h"
#include "../game/move.h"

namespace Strategies {

class Strategy {
 public:
  virtual Game::Move GetMove(const Game::Game& game) = 0;
};

}  // namespace Strategies