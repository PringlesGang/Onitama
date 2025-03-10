#pragma once

#include "../game/card.h"
#include "command.h"

namespace Cli {

void ExecuteCards();

class CardsCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetHelp() const override;

 private:
  std::string GetCommand() const override;
};

}  // namespace Cli
