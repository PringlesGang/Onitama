#include "version.h"

#include <format>
#include <iostream>

namespace Cli {

void ExecuteVersion() { std::cout << std::format("v{}", VERSION) << std::endl; }

std::optional<Thunk> VersionCommand::Parse(std::istringstream& command) const {
  if (!Terminate(command)) return std::nullopt;
  return ExecuteVersion;
}

std::string VersionCommand::GetName() const { return "version"; }

std::string VersionCommand::GetCommand() const { return GetName(); }

std::string VersionCommand::GetHelp() const {
  return "Prints the current version of the software.";
}

}  // namespace Cli
