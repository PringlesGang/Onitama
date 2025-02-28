#pragma once

#include "../gameMaster.h"
#include "command.h"
#include "strategies.h"

namespace Cli {

struct GameArgs {
  StrategyFactory RedStrategy;
  StrategyFactory BlueStrategy;

  size_t Width = 5;
  size_t Height = 5;

  size_t RepeatCount = 1;
  bool RepeatCards = false;
  bool Multithread = false;

  PrintType GameArgsPrintType = PrintType::Board;

  std::optional<std::array<Game::Card, CARD_COUNT>> Cards;
};

struct ExecuteGameInfo {
  std::pair<size_t, size_t> Wins;
};

ExecuteGameInfo ExecuteGame(const GameArgs args);

class GameCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;

 private:
  static bool ParseCards(std::istringstream& command, GameArgs& args);
  static bool ParsePrintType(std::istringstream& command, GameArgs& args);
  static bool ParseOptionalArgs(std::istringstream& command, GameArgs& args);
};

}  // namespace Cli
