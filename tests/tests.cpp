#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>

#include "./game/game.h"

namespace Tests {

const static std::unordered_map<std::string, std::function<int()>> Tests = {
    {"initial game state constructor", Game::InitialStateConstructor},
    {"game from board constructor", Game::BoardConstructor},
    {"game copy constructor", Game::CopyConstructor},
    {"game move constructor", Game::MoveConstructor},
    {"game with random cards constructor", Game::WithRandomCards},
    {"game from serialization constructor", Game::FromSerialization},
    {"parse game serialization", Game::ParseSerialization},
    {"parse incorrect game serialization", Game::ParseIncorrectSerialization},
    {"game equality", Game::Equality},
    {"game inequality", Game::Inequality},
    {"get game hand", Game::GetHand},
    {"get current game hand", Game::GetCurrentHand},
    {"get set aside card", Game::GetSetAsideCard},
    {"get current player", Game::GetCurrentPlayer},
    {"get game dimensions", Game::GetDimensions},
};

int RunAll() {
  int exitCode = 0;
  for (const auto& [id, test] : Tests) {
    std::cout << std::format("Performing test \"{}\"... \t", id);
    const int result = test();
    std::cout << (result ? "Failed!" : "Passed!") << std::endl;
    exitCode |= result;
  }

  return exitCode;
}

int Run(const std::string& id) {
  if (!Tests.contains(id)) {
    std::cerr << std::format("Unknown test \"{}\"!", id) << std::endl;
    return 1;
  }

  return Tests.at(id)();
}

void Init() { Game::Init(); }

}  // namespace Tests

int main(int argc, char* argv[]) {
  if (argc <= 1) {
    std::cerr << "No test provided!" << std::endl;
    return 1;
  }

  Tests::Init();

  std::string command = "";
  for (int arg = 1; arg < argc; arg++) {
    if (arg != 1) command += " ";
    command += std::string(argv[arg]);
  }
  std::transform(command.begin(), command.end(), command.begin(),
                 [](unsigned char c) { return std::tolower(c); });

  if (command == "--all") return Tests::RunAll();

  return Tests::Run(command);
}
