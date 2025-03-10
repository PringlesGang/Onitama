#include "command.h"

#include <format>
#include <iostream>

namespace Cli {

bool Command::Terminate(std::istringstream& stream) {
  const bool terminated = Parse::Terminate(stream);

  if (!terminated) {
    std::string remaining;
    stream >> remaining;
    Parse::Unparse(stream, remaining);

    std::cerr << std::format("Unknown argument \"{}\"", remaining) << std::endl;
  }

  return terminated;
}

}  // namespace Cli
