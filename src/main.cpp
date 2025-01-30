#include <iostream>

#include "cli.h"

void WelcomeMessage() {
  std::cout << "Onitama analyzer v0.1.0" << std::endl << std::endl;
}

void CommandLoop() {
  while (true) {
    std::optional<std::function<void()>> command;
    do {
      std::string input;
      std::getline(std::cin, input);

      if (input == "exit" || input == "e") return;

      command = Cli::Parse(input);
    } while (!command);

    (*command)();
    std::cout << std::endl;
  }
}

int main(int argc, char* argv[]) {
  WelcomeMessage();

  if (argc <= 1) {
    CommandLoop();
  } else {
    std::string argument = "";
    for (size_t i = 1; i < argc; i++) {
      if (i != 1) argument += " ";
      argument += std::string(argv[i]);
    }

    std::optional<std::function<void()>> command = Cli::Parse(argument);
    if (command) (*command)();
  }

  return 0;
}