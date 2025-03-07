#include "experiment.h"

#include <array>
#include <format>
#include <iostream>

#include "../experiments/fairCards.h"
#include "../experiments/stateGraph.h"

namespace Cli {

const static std::array<ExperimentParser, 2> Experiments = {
    ExperimentParser{
        .Name = Experiments::FairCards::Name,
        .Parser = Experiments::FairCards::Parse,
        .Command = Experiments::FairCards::Command,
        .Description = Experiments::FairCards::Description,
    },
    ExperimentParser{
        .Name = Experiments::StateGraph::Name,
        .Parser = Experiments::StateGraph::Parse,
        .Command = Experiments::StateGraph::Command,
        .Description = Experiments::StateGraph::Description,
    },
};

void ExecuteListExperiments() {
  for (const ExperimentParser& experiment : Experiments) {
    std::cout << std::format("- {}\n{}\n\n", experiment.Name,
                             experiment.Description)
              << std::endl;
  }
}

std::optional<Thunk> ExperimentCommand::Parse(
    std::istringstream& command) const {
  std::string arg;
  if (!(command >> arg)) {
    std::cout << "Failed to parse experiment argument!" << std::endl;
    return std::nullopt;
  }
  ToLower(arg);

  if (arg == "--list" || arg == "-l") {
    if (!Terminate(command)) return std::nullopt;
    return ExecuteListExperiments;
  }

  for (const ExperimentParser& experiment : Experiments) {
    if (arg != experiment.Name) continue;

    const std::optional<Thunk> result = experiment.Parser(command);

    if (!result) std::cout << experiment.Command << std::endl;
    if (!Terminate(command)) return std::nullopt;

    return result;
  }

  std::cout << std::format("Invalid experiment name \"{}\"!", arg) << std::endl;
  return std::nullopt;
}

std::string ExperimentCommand::GetName() const { return "experiment"; }

std::string ExperimentCommand::GetCommand() const {
  return std::format("{} --list\n{} experiment", GetName(), GetName());
}

std::string ExperimentCommand::GetHelp() const {
  return "Performs the provided experiment, or lists all experiments if "
         "`--list` is used.";
}

}  // namespace Cli
