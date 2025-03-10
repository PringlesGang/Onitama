#include "print.h"

#include <iostream>

#include "../util/base64.h"

namespace Cli {

void ExecutePrintGame(const Game::GameSerialization game) {
  std::cout << Game::Game::FromSerialization(game) << std::endl;
}

std::optional<Thunk> PrintCommand::Parse(std::istringstream& command) const {
  std::string subCommand;
  command >> subCommand;
  Parse::ToLower(subCommand);

  if (subCommand.empty()) {
    std::cout << "No print subcommand provided!" << std::endl;
    return std::nullopt;
  }

  if (subCommand == "game") {
    return ParsePrintGame(command);
  } else {
    std::cout << std::format("Unknown subcommand \"{}\"", subCommand)
              << std::endl;
    return std::nullopt;
  }
}

std::optional<Thunk> PrintCommand::ParsePrintGame(
    std::istringstream& command) const {
  const std::optional<Game::GameSerialization> serialization =
      Game::Game::ParseSerialization(command);
  if (!serialization) return std::nullopt;

  if (!Terminate(command)) return std::nullopt;

  return [serialization] { ExecutePrintGame(serialization.value()); };
}

std::string PrintCommand::GetName() const { return "print"; }

std::string PrintCommand::GetCommand() const {
  return std::format("{} game serialization", GetName());
}

std::string PrintCommand::GetHelp() const {
  return "- game\n"
         "Prints the provided serialization as a game state.";
}

}  // namespace Cli
