#pragma once

#include "../util/parse.h"
#include "random.h"
#include "strategy.h"

namespace Strategy {

class MonteCarlo : public Strategy {
 public:
  MonteCarlo(size_t repeatCount);

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<MonteCarlo>()>> Parse(
      std::istringstream& stream);

  constexpr static std::string GetName() { return "montecarlo"; }

  constexpr static std::string GetHelpEntry() {
    constexpr std::string_view name = "MonteCarlo <repeat_count>";
    constexpr std::array<std::string_view, 2> description{
        "Recursively simulate the game to find a perfect move", "to play."};
    return Parse::PadCommandName(name, description);
  }

 private:
  Random RandomStrategy;

  const size_t RepeatCount;

  Color RunSimulation(Game::Game& game);
};

}  // namespace Strategy
