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

  std::istringstream lowered = std::istringstream(ToLower(command.str()));
  lowered >> option;

  if (option.empty()) {
    std::cout << "Failed to parse command!" << std::endl;
  } else {
    if (option == "help") return std::bind(&Cli::ExecuteHelp, this);

    for (auto command = commands.begin(); command != commands.end();
         command++) {
      if (option != (*command)->GetName()) continue;

      const std::optional<Thunk> result = (*command)->Parse(lowered);

      if (!result) std::cout << (*command)->GetCommand() << std::endl;

      return result;
    }

    std::cout << std::format("Unknown command \"{}\"!", option) << std::endl;
  }

  std::cout << GetCommand() << std::endl;

  return std::nullopt;
}

void Cli::ExecuteHelp() const { std::cout << GetHelp() << std::endl; }

}  // namespace Cli
