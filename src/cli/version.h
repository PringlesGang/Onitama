#pragma once

#include "command.h"

namespace Cli {

void ExecuteVersion();

class VersionCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "version"; }

  constexpr std::string GetHelpEntry() const override {
    constexpr std::array<std::string_view, 1> description{
        "Print the current version of the software."};
    return PadCommandName(GetName(), description);
  }

  constexpr std::string GetHelp() const override { return GetHelpEntry(); }
};

}  // namespace Cli
