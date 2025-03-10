#pragma once

#include <functional>
#include <optional>
#include <sstream>

#include "../util/parse.h"

namespace Cli {

typedef std::function<void()> Thunk;

class Command {
 public:
  virtual ~Command() = default;

  virtual std::optional<Thunk> Parse(std::istringstream& command) const = 0;

  virtual std::string GetName() const = 0;
  virtual std::string GetCommand() const = 0;
  virtual std::string GetHelp() const = 0;

  static bool Terminate(std::istringstream& stream);
};

}  // namespace Cli
