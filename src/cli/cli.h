#pragma once

#include "cards.h"
#include "command.h"
#include "experiment.h"
#include "game.h"
#include "print.h"
#include "strategies.h"
#include "version.h"

namespace Cli {

class Cli : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return ""; }
  constexpr std::string GetHelpEntry() const override { return ""; }

  constexpr std::string GetHelp() const override {
    std::string string = "";
    for (const auto& command : Commands) {
      string += command->GetHelpEntry();
    }
    string += Parse::PadCommandName("exit", "Exit the application.");

    return string;
  }

  void ExecuteHelp() const;

 private:
  const std::array<const std::unique_ptr<const Command>, 6> Commands = {
      std::make_unique<CardsCommand>(),
      std::make_unique<GameCommand>(),
      std::make_unique<StrategiesCommand>(),
      std::make_unique<ExperimentCommand>(),
      std::make_unique<PrintCommand>(),
      std::make_unique<VersionCommand>(),
  };
};

}  // namespace Cli
