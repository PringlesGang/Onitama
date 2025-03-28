#include "test.h"

#include <iostream>

#include "../../tests/tests.h"

namespace Cli {

std::optional<Thunk> TestCommand::Parse(std::istringstream& command) const {
  std::string string;
  command >> string;

  if (string == "--all") {
    if (!Parse::Terminate(command)) return std::nullopt;
    return Tests::RunAll;
  }

  Parse::Unparse(command, string);
  size_t testId;
  if (!(command >> testId)) {
    std::cerr << std::format("Unknown argument \"{}\"!", string) << std::endl;
    return std::nullopt;
  }

  if (!Parse::Terminate(command)) return std::nullopt;

  return [testId] { Tests::Run(testId); };
}

std::string TestCommand::GetName() const { return "test"; }

std::string TestCommand::GetCommand() const {
  return "test --all\n"
         "test test-id";
}

std::string TestCommand::GetHelp() const {
  return "Executes all unit tests or the unit test provided.";
}

}  // namespace Cli