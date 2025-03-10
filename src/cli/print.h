#pragma once

#include "../game/game.h"
#include "command.h"

namespace Cli {

void ExecutePrintGame(const Game::GameSerialization game);

class PrintCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;

 private:
  std::optional<Thunk> ParsePrintGame(std::istringstream& command) const;
};

}  // namespace Cli
