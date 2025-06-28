#include "experiment.h"

#include <array>
#include <format>
#include <iostream>

#include "../util/parse.h"

namespace Cli {

std::optional<Thunk> ExperimentCommand::Parse(
    std::istringstream& command) const {
  std::string arg;
  if (!(command >> arg)) {
    std::cout << "Failed to parse experiment argument!" << std::endl;
    return std::nullopt;
  }
  Parse::ToLower(arg);

  for (const ExperimentParser& experiment : Experiments) {
    if (arg != experiment.Name) continue;

    const std::optional<Thunk> result = experiment.Parser(command);

    if (!result) std::cout << experiment.Help << std::endl;
    if (!Terminate(command)) return std::nullopt;

    return result;
  }

  std::cout << std::format("Invalid experiment name \"{}\"!", arg) << std::endl;
  return std::nullopt;
}

}  // namespace Cli
