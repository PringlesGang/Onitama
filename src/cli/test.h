#pragma once

#include "command.h"

namespace Cli {

class TestCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;
};

}  // namespace Cli
