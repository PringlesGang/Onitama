#include "cli.h"

#include <format>
#include <iostream>

namespace Cli {

std::string Cli::GetName() const { return ""; }

std::string Cli::GetCommand() const {
  return "Type \"help\" for more information.";
}

std::string Cli::GetHelp() const {
  std::string string = "";

  for (auto command = commands.begin(); command != commands.end(); command++) {
    string += std::format("- {}\n{}\n\n", (*command)->GetCommand(),
                          (*command)->GetHelp());
  }

  return string;
}

std::optional<Thunk> Cli::Parse(std::istringstream& command) const {
  std::string option;

  command >> option;
  ToLower(option);

  if (option.empty()) {
    std::cout << "Failed to parse command!" << std::endl;
  } else {
    if (option == "help") return std::bind(&Cli::ExecuteHelp, this);

    for (auto commandIt = commands.begin(); commandIt != commands.end();
         commandIt++) {
      if (option != (*commandIt)->GetName()) continue;

      const std::optional<Thunk> result = (*commandIt)->Parse(command);

      if (!result) std::cout << (*commandIt)->GetCommand() << std::endl;

      return result;
    }

    std::cout << std::format("Unknown command \"{}\"!", option) << std::endl;
  }

  std::cout << GetCommand() << std::endl;

  return std::nullopt;
}

void Cli::ExecuteHelp() const { std::cout << GetHelp() << std::endl; }

}  // namespace Cli
