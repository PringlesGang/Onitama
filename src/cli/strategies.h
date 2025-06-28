#pragma once

#include <memory>

#include "../strategies/strategy.h"
#include "command.h"

namespace Cli {

void ExecuteStrategies();

typedef std::function<std::unique_ptr<Strategy::Strategy>()> StrategyFactory;
std::optional<StrategyFactory> ParseStrategy(std::istringstream& command);

struct StrategyParser {
  const std::string Name;
  const std::function<std::optional<StrategyFactory>(std::istringstream&)>
      Parser;
  const std::string HelpEntry;
};

class StrategiesCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "strategies"; }

  constexpr std::string GetHelpEntry() const override {
    return Parse::PadCommandName(GetName(), "Prints all available strategies.");
  }

  constexpr std::string GetHelp() const override { return GetHelpEntry(); }
};

}  // namespace Cli
