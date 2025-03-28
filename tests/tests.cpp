#include "tests.h"

#include <format>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>

#include "./game/game.h"

namespace Tests {

const static std::unordered_map<std::size_t, std::function<int()>> Tests = {
    {0, Game::InitialStateConstructor},
};

void RunAll() {
  int exitCode = 0;
  for (const auto& [id, test] : Tests) {
    std::cout << std::format("Performing test {}... \t", id);
    const int result = test();
    std::cout << (result ? "Failed!" : "Passed!") << std::endl;
    exitCode |= result;
  }

  exit(exitCode);
}

void Run(size_t id) {
  if (!Tests.contains(id)) {
    std::cerr << std::format("Unknown test {}!", id) << std::endl;
    exit(1);
  }

  exit(Tests.at(id)());
}

}  // namespace Tests
