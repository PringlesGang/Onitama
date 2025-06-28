#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <sstream>

#include "../util/parse.h"
#include "strategy.h"

namespace Strategy {

class Human : public Strategy {
 public:
  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Human>()>> Parse(
      std::istringstream& stream);

  constexpr static std::string GetName() { return "human"; }

  constexpr static std::string GetHelpEntry() {
    return Parse::PadCommandName(
        "Human", "Play the game yourself through command-line input.");
  }

 private:
  static std::optional<Game::Move> ParseMove(std::istringstream& input,
                                             const Game::Game& game);
  static std::optional<Game::Move> ParseCard(std::istringstream& input,
                                             const Game::Game& game);
};

}  // namespace Strategy
