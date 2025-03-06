#include "command.h"

#include <format>
#include <iostream>

namespace Cli {

void Command::ToLower(std::string& string) {
  for (char& character : string) character = tolower(character);
}

void Command::Unparse(std::istringstream& stream, const std::string& string) {
  for (size_t i = 0; i < string.size(); i++) {
    stream.unget();
  }
}

bool Command::Terminate(std::istringstream& command) {
  std::string remaining;
  command >> remaining;

  if (remaining.empty()) return true;

  std::cout << std::format("Unknown argument \"{}\"!", remaining) << std::endl;
  return false;
}

}  // namespace Cli
