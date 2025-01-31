#pragma once

#include <optional>
#include <sstream>

#include "strategy.h"

namespace Strategy {

class Human : public Strategy {
 public:
  Game::Move GetMove(const Game::Game& game) override;

 private:
  static std::optional<Game::Move> ParseMove(std::istringstream& input,
                                             const Game::Game& game);
  static std::optional<Game::Move> ParseCard(std::istringstream& input,
                                             const Game::Game& game);
};

}  // namespace Strategy
