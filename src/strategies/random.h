#pragma once

#include "strategy.h"

namespace Strategy {

class Random : public Strategy {
 public:
  Game::Move GetMove(const Game::Game& game) override;
};

}  // namespace Strategy