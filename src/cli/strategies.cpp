#include "strategies.h"

#include <iostream>

#include "../strategies/human.h"
#include "../strategies/monteCarlo.h"
#include "../strategies/random.h"

namespace Cli {

void ExecuteStrategies() {
  const std::array<const std::string, 3> strategies = {
      "human",
      "random",
      "montecarlo",
  };

  for (const std::string& strategy : strategies) {
    std::cout << std::format("- {}", strategy) << std::endl;
  }
}

std::optional<StrategyFactory> ParseStrategy(std::istringstream& command) {
  std::string name;
  if (!(command >> name)) {
    std::cout << "No strategy provided!" << std::endl;
    return std::nullopt;
  };

  if (name == "human") {
    return [] { return std::make_unique<Strategy::Human>(); };
  }

  if (name == "random") {
    return [] { return std::make_unique<Strategy::Random>(); };
  }

  if (name == "montecarlo") {
    return [] { return std::make_unique<Strategy::MonteCarlo>(50); };
  }

  std::cout << std::format("Invalid strategy name \"{}\"!", name) << std::endl;
  return std::nullopt;
}

std::string StrategiesCommand::GetName() const { return "strategies"; }

std::string StrategiesCommand::GetCommand() const { return GetName(); }

std::string StrategiesCommand::GetHelp() const {
  return "Prints all available strategies to the console.";
}

std::optional<Thunk> StrategiesCommand::Parse(
    std::istringstream& command) const {
  if (!Terminate(command)) return std::nullopt;

  return ExecuteStrategies;
}

}  // namespace Cli
