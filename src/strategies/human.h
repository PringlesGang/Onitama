#pragma once

#include <optional>

#include "strategy.h"

namespace Strategy {

class Human : public Strategy {
 public:
  Game::Move GetMove(const Game::Game& game) override;

 private:
  static std::optional<Game::Move> ParseMove(const std::string& string,
                                             const Game::Game& game);
  static std::optional<Game::Move> ParseCard(const std::string& string,
                                             const Game::Game& game);
};

}  // namespace Strategy
