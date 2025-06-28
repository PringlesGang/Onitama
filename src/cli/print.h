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

  constexpr std::string GetName() const override { return "print"; }

  constexpr std::string GetHelpEntry() const override {
    return PadCommandName(GetName(), "Prints the provided game.");
  }

  constexpr std::string GetHelp() const override {
    return "print game <game_id> [--image <image_path>]\n"
           "\n"
           "Prints the provided game. Exports an image to the\n"
           "provided path if `--image` is given.\n";
  }

 private:
  std::optional<Thunk> ParsePrintGame(std::istringstream& command) const;
  bool ParsePrintGameArgs(std::istringstream& command,
                          PrintGameArgs& args) const;
};

}  // namespace Cli
