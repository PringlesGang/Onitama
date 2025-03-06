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
  ToLower(subCommand);

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
      ParseGameSerialization(command);
  if (!serialization) return std::nullopt;

  if (!Terminate(command)) return std::nullopt;

  return [serialization] { ExecutePrintGame(serialization.value()); };
}

std::optional<Game::GameSerialization> PrintCommand::ParseGameSerialization(
    std::istringstream& command) const {
  std::string string;
  command >> string;

  if (string.empty()) {
    std::cout << "No game serialization provided!" << std::endl;
    return std::nullopt;
  }

  const std::optional<Game::GameSerialization> serialization =
      Base64::Decode<Game::GAME_SERIALIZATION_SIZE>(string);
  if (!serialization) {
    std::cout << std::format("Invalid base64 game serialization \"{}\"!",
                             string)
              << std::endl;
    return std::nullopt;
  }

  return serialization;
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
