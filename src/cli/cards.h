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
    constexpr std::array<std::string_view, 1> description{
        "Prints all available cards."};
    return PadCommandName(GetName(), description);
  }

  constexpr std::string GetHelp() const override { return GetHelpEntry(); }
};

}  // namespace Cli
