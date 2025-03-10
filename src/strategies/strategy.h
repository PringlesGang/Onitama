#pragma once

#include "../game/game.h"
#include "../game/move.h"

namespace Strategy {

class Strategy {
 public:
  virtual ~Strategy() = default;

  virtual Game::Move GetMove(const Game::Game& game) = 0;
};

}  // namespace Strategy