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

  virtual constexpr std::string GetName() const = 0;
  virtual constexpr std::string GetHelpEntry() const = 0;
  virtual constexpr std::string GetHelp() const = 0;

  static bool Terminate(std::istringstream& stream);

 protected:
  static constexpr size_t HelpPaddingSize = 24;
  static constexpr std::string PadCommandName(
      const std::string_view name,
      const std::span<const std::string_view> help) {
    const size_t nameLength = name.size();

    std::string string = std::string(name);
    if (nameLength >= HelpPaddingSize - 2) {
      string += "\n" + std::string(HelpPaddingSize, ' ') +
                std::string(help[0]) + "\n";
    } else {
      string += std::string(HelpPaddingSize - nameLength, ' ') +
                std::string(help[0]) + "\n";
    }

    for (size_t i = 1; i < help.size(); i++) {
      string += std::string(help[i]) + "\n";
    }

    return string;
  }

  static constexpr std::string PadCommandName(
      const std::string_view name, const std::string_view description) {
    return PadCommandName(name, std::array<std::string_view, 1>{description});
  }
};

}  // namespace Cli
