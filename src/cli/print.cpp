#include "print.h"

#include <iostream>

#include "../util/base64.h"

namespace Cli {

void ExecutePrintGame(const PrintGameArgs args) {
  const Game::Game game = Game::Game::FromSerialization(args.Serialization);

  if (!args.ImagePath) {
    std::cout << game << std::endl;
  } else {
    game.ExportImage(args.ImagePath.value());
  }
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

bool PrintCommand::ParsePrintGameArgs(std::istringstream& command,
                                      PrintGameArgs& args) const {
  std::string argument;
  command >> argument;

  if (argument == "--image") {
    std::string path;
    command >> path;

    args.ImagePath = std::filesystem::path(path);
  } else {
    Parse::Unparse(command, argument);
    return true;
  }

  return ParsePrintGameArgs(command, args);
}

std::optional<Thunk> PrintCommand::ParsePrintGame(
    std::istringstream& command) const {
  if (Parse::ParseHelp(command))
    return [this] { std::cout << GetHelp() << std::endl; };

  const std::optional<Game::GameSerialization> serialization =
      Game::Game::ParseSerialization(command);
  if (!serialization) return std::nullopt;

  PrintGameArgs args{.Serialization = serialization.value()};

  if (!ParsePrintGameArgs(command, args)) return std::nullopt;

  if (!Terminate(command)) return std::nullopt;

  return [args] { ExecutePrintGame(args); };
}

}  // namespace Cli
