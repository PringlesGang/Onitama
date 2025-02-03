#pragma once

#include "command.h"
#include "strategies.h"

namespace Cli {

#pragma warning(suppress : 4091)
static struct GameArgs {
  StrategyFactory RedStrategy;
  StrategyFactory BlueStrategy;

  size_t RepeatCount = 1;
  bool RepeatCards = false;

  std::optional<std::array<Game::Card, CARD_COUNT>> Cards;
};

void ExecuteGame(const GameArgs args);

class GameCommand : public Command {
 public:
  std::optional<Thunk> Parse(std::istringstream& command) const override;

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;

 private:
  static bool ParseCards(std::istringstream& command, GameArgs& args);
  static bool ParseOptionalArgs(std::istringstream& command, GameArgs& args);
};

}  // namespace Cli
