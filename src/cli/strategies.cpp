#include "strategies.h"

#include <array>
#include <format>
#include <iostream>

#include "../strategies/human.h"
#include "../strategies/minMax.h"
#include "../strategies/monteCarlo.h"
#include "../strategies/positional.h"
#include "../strategies/random.h"
#include "../util/parse.h"

namespace Cli {

const static std::array<StrategyParser, 5> Strategies = {
    StrategyParser{.Name = Strategy::Human::GetName(),
                   .Parser = Strategy::Human::Parse,
                   .Command = Strategy::Human::GetCommand(),
                   .Description = Strategy::Human::GetDescription()},

    StrategyParser{.Name = Strategy::Random::GetName(),
                   .Parser = Strategy::Random::Parse,
                   .Command = Strategy::Random::GetCommand(),
                   .Description = Strategy::Random::GetDescription()},

    StrategyParser{.Name = Strategy::MonteCarlo::GetName(),
                   .Parser = Strategy::MonteCarlo::Parse,
                   .Command = Strategy::MonteCarlo::GetCommand(),
                   .Description = Strategy::MonteCarlo::GetDescription()},

    StrategyParser{.Name = Strategy::MinMax::GetName(),
                   .Parser = Strategy::MinMax::Parse,
                   .Command = Strategy::MinMax::GetCommand(),
                   .Description = Strategy::MinMax::GetDescription()},

    StrategyParser{.Name = Strategy::Positional::GetName(),
                   .Parser = Strategy::Positional::Parse,
                   .Command = Strategy::Positional::GetCommand(),
                   .Description = Strategy::Positional::GetDescription()},
};

void ExecuteStrategies() {
  for (const StrategyParser& strategy : Strategies) {
    std::cout << std::format("- {}\n{}\n\n", strategy.Name,
                             strategy.Description)
              << std::endl;
  }
}

std::optional<StrategyFactory> ParseStrategy(std::istringstream& command) {
  std::string name;
  if (!(command >> name)) {
    std::cout << "No strategy provided!" << std::endl;
    return std::nullopt;
  };
  Parse::ToLower(name);

  for (const StrategyParser& strategy : Strategies) {
    if (name != strategy.Name) continue;

    const std::optional<StrategyFactory> result = strategy.Parser(command);

    if (!result) std::cout << strategy.Command << std::endl;

    return result;
  }

  std::cout << std::format("Invalid strategy name \"{}\"!", name) << std::endl;
  return std::nullopt;
}

std::optional<Thunk> StrategiesCommand::Parse(
    std::istringstream& command) const {
  if (!Terminate(command)) return std::nullopt;

  return ExecuteStrategies;
}

}  // namespace Cli
