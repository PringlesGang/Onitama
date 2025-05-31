#include <iostream>

#include "cli/cli.h"

void WelcomeMessage() {
  std::cout << "Onitama analyzer v1.0.1" << std::endl << std::endl;
}

void CommandLoop() {
  Cli::Cli cli;

  while (true) {
    std::optional<std::function<void()>> command;
    do {
      std::cout << "> ";

      std::string input;
      std::getline(std::cin, input);

      if (input == "exit" || input == "e") return;

      std::istringstream stream = std::istringstream(input);
      command = cli.Parse(stream);
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
    Cli::Cli cli;

    std::string argument = "";
    for (size_t i = 1; i < argc; i++) {
      if (i != 1) argument += " ";
      argument += std::string(argv[i]);
    }

    std::istringstream stream(argument);
    std::optional<Cli::Thunk> command = cli.Parse(stream);

    if (command) (*command)();
  }

  return 0;
}