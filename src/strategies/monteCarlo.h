#pragma once

#include "random.h"
#include "strategy.h"

namespace Strategy {

class MonteCarlo : public Strategy {
 public:
  MonteCarlo(size_t repeatCount);

  Game::Move GetMove(const Game::Game& game) override;

 private:
  Random RandomStrategy;

  const size_t RepeatCount;

  Color RunSimulation(Game::Game& game);
};

}  // namespace Strategy
