#pragma once

#include <functional>

#include "../util/winState.h"
#include "strategy.h"

namespace Strategy {

class MinMax : public Strategy {
 public:
  MinMax(const std::optional<const size_t> maxDepth);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<MinMax>()>> Parse(
      std::istringstream& stream);

  static std::string GetName();
  static std::string GetCommand();
  static std::string GetDescription();

 private:
  const std::optional<const size_t> MaxDepth;

  WinState PlayRecursive(Game::Game game, const size_t depth) const;
};

}  // namespace Strategy
