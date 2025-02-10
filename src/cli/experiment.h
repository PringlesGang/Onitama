#pragma once

#include "command.h"

namespace Cli {

struct ExperimentParser {
  const std::string_view Name;
  const std::function<std::optional<Thunk>(std::istringstream&)> Parser;
  const std::string_view Command;
  const std::string_view Description;
};

void ExecuteListExperiments();

class ExperimentCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;
};

}  // namespace Cli
