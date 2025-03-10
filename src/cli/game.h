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

  std::string GetName() const override;
  std::string GetCommand() const override;
  std::string GetHelp() const override;
};

}  // namespace Cli
