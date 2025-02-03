#pragma once

#include <memory>

#include "../strategies/strategy.h"
#include "command.h"

namespace Cli {

void ExecuteStrategies();

typedef std::function<std::unique_ptr<Strategy::Strategy>()> StrategyFactory;
std::optional<StrategyFactory> ParseStrategy(std::istringstream& command);

#pragma warning(suppress : 4091)
static struct StrategyParser {
  const std::string Name;
  const std::function<std::optional<StrategyFactory>(std::istringstream&)>
      Parser;
  const std::string Command;
  const std::string Description;
};

class StrategiesCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;
};

}  // namespace Cli
