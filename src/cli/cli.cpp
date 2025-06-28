#include "cli.h"

#include <format>
#include <iostream>

#include "../util/parse.h"

namespace Cli {

std::optional<Thunk> Cli::Parse(std::istringstream& inputCommand) const {
  std::string option;

  inputCommand >> option;
  Parse::ToLower(option);

  if (option.empty()) {
    std::cout << "Failed to parse command!" << std::endl;
  } else {
    if (option == "help") return std::bind(&Cli::ExecuteHelp, this);

    for (const auto& command : Commands) {
      if (option != command->GetName()) continue;

      const std::optional<Thunk> result = command->Parse(inputCommand);

      return result;
    }

    std::cout << std::format("Unknown command \"{}\"!", option) << std::endl;
  }

  return std::nullopt;
}

void Cli::ExecuteHelp() const { std::cout << GetHelp() << std::endl; }

}  // namespace Cli
