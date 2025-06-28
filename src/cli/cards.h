#pragma once

#include "../game/card.h"
#include "command.h"

namespace Cli {

void ExecuteCards();

class CardsCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "cards"; }

  constexpr std::string GetHelpEntry() const override {
    return Parse::PadCommandName(GetName(), "Prints all available cards.");
  }

  constexpr std::string GetHelp() const override { return GetHelpEntry(); }
};

}  // namespace Cli
