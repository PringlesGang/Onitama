#pragma once

#include <functional>

#include "../util/parse.h"
#include "../util/winState.h"
#include "strategy.h"

namespace Strategy {

class MinMax : public Strategy {
 public:
  MinMax(const std::optional<const size_t> maxDepth);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<MinMax>()>> Parse(
      std::istringstream& stream);

  constexpr static std::string GetName() { return "minmax"; }

  constexpr static std::string GetHelpEntry() {
    constexpr std::string_view name = "MinMax [<max_depth>] [--no-max-depth]";
    constexpr std::array<std::string_view, 2> description{
        "Recursively simulate the game to find a perfect move", "to play."};
    return Parse::PadCommandName(name, description);
  }

 private:
  const std::optional<const size_t> MaxDepth;

  WinState PlayRecursive(Game::Game game, const size_t depth) const;
};

}  // namespace Strategy
