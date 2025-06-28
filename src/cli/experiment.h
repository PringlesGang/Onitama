#pragma once

#include "../experiments/fairCards/fairCards.h"
#include "../experiments/stateGraph/stateGraph.h"
#include "command.h"

namespace Cli {

struct ExperimentParser {
  const std::string_view Name;
  const std::function<std::optional<Thunk>(std::istringstream&)> Parser;
  const std::string_view HelpEntry;
  const std::string_view Help;
};

const inline std::array<ExperimentParser, 2> Experiments = {
    ExperimentParser{
        .Name = Experiments::FairCards::Name,
        .Parser = Experiments::FairCards::Parse,
        .HelpEntry = Experiments::FairCards::HelpEntry,
        .Help = Experiments::FairCards::Help,
    },
    ExperimentParser{
        .Name = Experiments::StateGraph::Name,
        .Parser = Experiments::StateGraph::Parse,
        .HelpEntry = Experiments::StateGraph::HelpEntry,
        .Help = Experiments::StateGraph::Help,
    },
};

void ExecuteListExperiments();

class ExperimentCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "experiment"; }

  constexpr std::string GetHelpEntry() const override {
    return Parse::PadCommandName(GetName(), "Performs an experiment.");
  }

  constexpr std::string GetHelp() const override {
    std::string help = "Performs an experiment.\n\n";
    for (const ExperimentParser& experiment : Experiments) {
      help += experiment.HelpEntry;
    }
    return help;
  };
};

}  // namespace Cli
