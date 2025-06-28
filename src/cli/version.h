#pragma once

#include "command.h"

namespace Cli {

void ExecuteVersion();

class VersionCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "version"; }

  constexpr std::string GetHelpEntry() const override {
    return Parse::PadCommandName(GetName(),
                                 "Print the current version of the software.");
  }

  constexpr std::string GetHelp() const override { return GetHelpEntry(); }
};

}  // namespace Cli
