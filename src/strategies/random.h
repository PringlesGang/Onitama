#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <sstream>

#include "strategy.h"

namespace Strategy {

class Random : public Strategy {
 public:
  Random();

  Game::Move GetMove(const Game::Game& game) override;

  static std::optional<std::function<std::unique_ptr<Random>()>> Parse(
      std::istringstream& command);

  static std::string GetName();
  static std::string GetCommand();
  static std::string GetDescription();

 private:
  std::random_device RandomDevice;
  std::mt19937 Generator;
};

}  // namespace Strategy