#pragma once

#include "cards.h"
#include "command.h"
#include "experiment.h"
#include "game.h"
#include "strategies.h"

namespace Cli {

class Cli : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;

  void ExecuteHelp() const;

 private:
  const std::array<const std::unique_ptr<const Command>, 4> commands = {
      std::make_unique<CardsCommand>(),
      std::make_unique<GameCommand>(),
      std::make_unique<StrategiesCommand>(),
      std::make_unique<ExperimentCommand>(),
  };
};

}  // namespace Cli
