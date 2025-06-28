#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <sstream>

#include "../util/parse.h"
#include "strategy.h"

namespace Strategy {

class Random : public Strategy {
 public:
  Random();

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Random>()>> Parse(
      std::istringstream& stream);

  constexpr static std::string GetName() { return "random"; }

  constexpr static std::string GetHelpEntry() {
    return Parse::PadCommandName(
        "Random", "Choose a random valid move, uniformly distributed.");
  }

 private:
  std::random_device RandomDevice;
  std::mt19937 Generator;
};

}  // namespace Strategy