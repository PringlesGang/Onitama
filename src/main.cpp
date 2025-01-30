#include <iostream>

#include "cli.h"

int main(int argc, char* argv[]) {
  while (true) {
    std::optional<std::function<void()>> command;
    do {
      std::string input;
      std::getline(std::cin, input);

      if (input == "exit" || input == "e") return 0;

      command = Cli::Parse(input);
    } while (!command);

    (*command)();
    std::cout << std::endl;
  }
}