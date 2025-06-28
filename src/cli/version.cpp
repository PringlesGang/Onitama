#include "version.h"

#include <format>
#include <iostream>

namespace Cli {

void ExecuteVersion() { std::cout << std::format("v{}", VERSION) << std::endl; }

std::optional<Thunk> VersionCommand::Parse(std::istringstream& command) const {
  if (Parse::ParseHelp(command))
    return [this] { std::cout << GetHelp() << std::endl; };

  if (!Terminate(command)) return std::nullopt;
  return ExecuteVersion;
}

}  // namespace Cli
