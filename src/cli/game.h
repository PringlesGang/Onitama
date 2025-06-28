#pragma once

#include "../gameMaster.h"
#include "../util/parse.h"
#include "command.h"
#include "strategies.h"

namespace Cli {

struct GameArgs {
 public:
  bool Parse(std::istringstream& stream);
  bool IsValid() const;

  StrategyFactory RedStrategy;
  StrategyFactory BlueStrategy;

  Parse::GameConfiguration Configuration;

  size_t RepeatCount = 1;
  bool Multithread = false;

  PrintType GameArgsPrintType = PrintType::Board;
};

struct ExecuteGameInfo {
  std::pair<size_t, size_t> Wins;
};

ExecuteGameInfo ExecuteGame(const GameArgs args);

class GameCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  constexpr std::string GetName() const override { return "game"; }

  constexpr std::string GetHelpEntry() const override {
    return PadCommandName(GetName(),
                          "Simulates a game with the provided strategies.");
  }

  constexpr std::string GetHelp() const override {
    return "game <red_strategy> <blue_strategy> [options]\n"
           "\n"
           "Simulates a game with the provided strategies.\n"
           "List all strategies with the `strategies` command.\n"
           "\n"
           "Options:\n"
           "-d, --duplicate-cards   Allow for duplicate cards to be drawn.\n"
           "-c, --cards             Provide a list of five cards to use, in\n"
           "                        the order of set-aside, red hand, blue\n"
           "                        hand. You can also provide a single card\n"
           "                        to be duplicated five times. If not\n"
           "                        supplied, the cards are chosen at random.\n"
           "-s, --size              Provide the width and height of the\n"
           "                        board, in that order. Width must be at\n"
           "                        least 1, and height must be at least 2.\n"
           "                        Default is 5x5.\n"
           "-n, --repeat            Repeat this configuration the provided\n"
           "                        number of times.\n"
           "-p, --print-type        Defines how the game is to be printed.\n"
           "                        - \"board\" prints the game board;\n"
           "                        - \"data\" prints the sequence of moves\n"
           "                          performed;\n"
           "                        - \"wins\" prints the outcome of each\n"
           "                          game and counts how many games were\n"
           "                          won by each player;\n"
           "                        - \"none\" doesn't print anything.\n"
           "                        \"board\" is the default.\n"
           "-m, --multithread       In conjunction with --repeat, plays all\n"
           "                        games simultaneously on separate\n"
           "                        threads.\n";
  }
};

}  // namespace Cli
