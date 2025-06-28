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
                   .HelpEntry = Strategy::Human::GetHelpEntry()},

    StrategyParser{.Name = Strategy::Random::GetName(),
                   .Parser = Strategy::Random::Parse,
                   .HelpEntry = Strategy::Random::GetHelpEntry()},

    StrategyParser{.Name = Strategy::MonteCarlo::GetName(),
                   .Parser = Strategy::MonteCarlo::Parse,
                   .HelpEntry = Strategy::MonteCarlo::GetHelpEntry()},

    StrategyParser{.Name = Strategy::MinMax::GetName(),
                   .Parser = Strategy::MinMax::Parse,
                   .HelpEntry = Strategy::MinMax::GetHelpEntry()},

    StrategyParser{.Name = Strategy::Positional::GetName(),
                   .Parser = Strategy::Positional::Parse,
                   .HelpEntry = Strategy::Positional::GetHelpEntry()},
};

void ExecuteStrategies() {
  std::string string = "";
  for (const StrategyParser& strategy : Strategies) {
    string += strategy.HelpEntry;
  }
  std::cout << string << std::endl;
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

    return result;
  }

  std::cout << std::format("Invalid strategy name \"{}\"!", name) << std::endl;
  return std::nullopt;
}

std::optional<Thunk> StrategiesCommand::Parse(
    std::istringstream& command) const {
  if (Parse::ParseHelp(command))
    return [this] { std::cout << GetHelp() << std::endl; };

  if (!Terminate(command)) return std::nullopt;

  return ExecuteStrategies;
}

}  // namespace Cli
