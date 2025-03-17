#pragma once

#include "../game/game.h"
#include "command.h"

namespace Cli {

struct PrintGameArgs {
  Game::GameSerialization Serialization;
  std::optional<std::filesystem::path> ImagePath = std::nullopt;
};

void ExecutePrintGame(const PrintGameArgs args);

class PrintCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;

 private:
  std::optional<Thunk> ParsePrintGame(std::istringstream& command) const;
  bool ParsePrintGameArgs(std::istringstream& command,
                          PrintGameArgs& args) const;
};

}  // namespace Cli
