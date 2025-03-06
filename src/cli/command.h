#pragma once

#include <functional>
#include <optional>
#include <sstream>

namespace Cli {

typedef std::function<void()> Thunk;

class Command {
 public:
  virtual std::optional<Thunk> Parse(std::istringstream& command) const = 0;

  virtual std::string GetName() const = 0;
  virtual std::string GetCommand() const = 0;
  virtual std::string GetHelp() const = 0;

  static void ToLower(std::string& string);
  static void Unparse(std::istringstream& stream, const std::string& string);
  static bool Terminate(std::istringstream& command);
};

}  // namespace Cli
