#pragma once

#include <random>

#include "strategy.h"

namespace Strategy {

class Random : public Strategy {
 public:
  Random();

  Game::Move GetMove(const Game::Game& game) override;

 private:
  std::random_device RandomDevice;
  std::mt19937 Generator;
};

}  // namespace Strategy